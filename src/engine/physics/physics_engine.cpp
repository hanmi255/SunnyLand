#include "physics_engine.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/tilelayer_component.h"
#include "../component/transform_component.h"
#include "../object/game_object.h"
#include "collision.h"
#include <algorithm>
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::physics {

    int64_t PhysicsEngine::SpatialGrid::getGridKey(float x, float y) const
    {
        int32_t grid_x = static_cast<int32_t>(std::floor(x * inv_cell_size));
        int32_t grid_y = static_cast<int32_t>(std::floor(y * inv_cell_size));
        // 使用64位避免哈希冲突，更安全的组合方式
        return (static_cast<int64_t>(grid_x) << 32) | static_cast<int64_t>(grid_y);
    }

    void PhysicsEngine::SpatialGrid::clear()
    {
        for (auto &[key, objects] : grid) {
            objects.clear(); // 清空但保留容量
        }
        // 不清除grid本身，避免重新分配哈希表
    }

    void PhysicsEngine::SpatialGrid::insert(engine::object::GameObject* obj,
                                            engine::component::ColliderComponent* cc)
    {
        auto* transform = obj->getComponent<engine::component::TransformComponent>();
        if (!transform) return;

        const auto world_aabb = cc->getWorldAABB();

        // 获取对象覆盖的所有网格
        auto grid_keys = getObjectGridKeys(world_aabb.position.x, world_aabb.position.y,
                                           world_aabb.size.x, world_aabb.size.y);

        for (int64_t key : grid_keys) {
            grid[key].emplace_back(obj, cc);
        }
    }

    std::vector<int64_t> PhysicsEngine::SpatialGrid::getNearbyKeys(float x, float y, float width,
                                                                   float height) const
    {
        // 扩展搜索范围以包含相邻网格
        const float margin = cell_size * 0.1f; // 小的边界扩展
        return getObjectGridKeys(x - margin, y - margin, width + 2 * margin, height + 2 * margin);
    }

    std::vector<int64_t>
    PhysicsEngine::SpatialGrid::getObjectGridKeys(float x, float y, float width, float height) const
    {
        std::vector<int64_t> keys;

        int32_t min_x = static_cast<int32_t>(std::floor(x * inv_cell_size));
        int32_t max_x = static_cast<int32_t>(std::floor((x + width) * inv_cell_size));
        int32_t min_y = static_cast<int32_t>(std::floor(y * inv_cell_size));
        int32_t max_y = static_cast<int32_t>(std::floor((y + height) * inv_cell_size));

        keys.reserve((max_x - min_x + 1) * (max_y - min_y + 1));

        for (int32_t gx = min_x; gx <= max_x; ++gx) {
            for (int32_t gy = min_y; gy <= max_y; ++gy) {
                keys.push_back((static_cast<int64_t>(gx) << 32) | static_cast<int64_t>(gy));
            }
        }

        return keys;
    }

    void PhysicsEngine::registerComponent(engine::component::PhysicsComponent* component)
    {
        components_.push_back(component);
        spdlog::trace("PhysicsComponent 注册完成。");
    }

    void PhysicsEngine::unregisterComponent(engine::component::PhysicsComponent* component)
    {
        // 使用 remove-erase 方法安全地移除指针
        auto it = std::remove(components_.begin(), components_.end(), component);
        components_.erase(it, components_.end());
        spdlog::trace("PhysicsComponent 注销完成。");
    }

    void
    PhysicsEngine::registerCollisionTileLayer(engine::component::TileLayerComponent* tile_layer)
    {
        tile_layer->setPhysicsEngine(this);
        collision_tile_layers_.push_back(tile_layer);
        spdlog::trace("TileLayer 注册完成。");
    }

    void
    PhysicsEngine::unregisterCollisionTileLayer(engine::component::TileLayerComponent* tile_layer)
    {
        // 使用 remove-erase 方法安全地移除指针
        auto it =
            std::remove(collision_tile_layers_.begin(), collision_tile_layers_.end(), tile_layer);
        collision_tile_layers_.erase(it, collision_tile_layers_.end());
        spdlog::trace("TileLayer 注销完成。");
    }

    void PhysicsEngine::update(float delta_time)
    {
        // 每帧开始时先清空碰撞对和瓦片触发事件列表
        collision_pairs_.clear();
        tile_trigger_events_.clear();

        // 遍历所有注册的物理组件
        for (auto* pc : components_) {
            if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
                continue;
            }

            pc->resetCollisionFlags(); // 重置碰撞标志

            // 应用重力 (如果组件受重力影响)：F = m * g
            if (pc->isUseGravity()) {
                pc->addForce(gravity_ * pc->getMass());
            }
            /* 还可以添加其它力影响，比如风力、摩擦力等，目前不考虑 */

            // 更新速度： v += a * dt，其中 a = F / m
            pc->velocity_ += (pc->getForce() / pc->getMass()) * delta_time;
            pc->clearForce(); // 清除当前帧的力

            // 处理瓦片碰撞
            resolveTileCollisions(pc, delta_time);

            // 应用世界边界
            applyWorldBounds(pc);
        }

        // 检查对象间碰撞
        checkObjectCollisions();

        // 检查瓦片触发事件
        checkTileTriggers();
    }

    void PhysicsEngine::checkObjectCollisions()
    {
        spatial_grid_.clear();

        // 第一步：将所有有效碰撞器插入空间网格
        for (auto* pc : components_) {
            if (!pc || !pc->isEnabled()) continue;

            auto* obj = pc->getOwner();
            if (!obj) continue;

            auto* cc = obj->getComponent<engine::component::ColliderComponent>();
            if (!cc || !cc->isActive()) continue;

            spatial_grid_.insert(obj, cc);
        }

        // 第二步：检查碰撞，避免重复检查
        std::set<std::pair<engine::object::GameObject*, engine::object::GameObject*>> checked_pairs;

        for (const auto &[grid_key, objects] : spatial_grid_.grid) {
            if (objects.size() < 2) continue; // 单个对象无需检查

            checkCollisionsInCell(objects, checked_pairs);
        }
    }

    void PhysicsEngine::checkTileTriggers()
    {
        tile_trigger_events_.clear();

        // 使用 GameObject* 和 TileType 的组合来避免重复触发
        std::set<std::pair<engine::object::GameObject*, engine::component::TileType>>
            triggered_pairs;

        for (auto* pc : components_) {
            if (!pc || !pc->isEnabled()) continue; // 检查组件是否有效和启用
            auto* obj = pc->getOwner();
            if (!obj) continue;
            auto* cc = obj->getComponent<engine::component::ColliderComponent>();
            if (!cc || !cc->isActive() || cc->isTrigger())
                continue; // 如果游戏对象本就是触发器，则不需要检查瓦片触发事件

            // 获取物体的世界AABB
            auto world_aabb = cc->getWorldAABB();

            // 遍历所有注册的碰撞瓦片层分别进行检测
            for (auto* layer : collision_tile_layers_) {
                if (!layer) continue;
                auto tile_size = layer->getTileSize();
                glm::vec2 layer_offset = layer->getOffset();

                constexpr float TOLERANCE = 1.0f; // 检查边缘时的容差值
                // 获取瓦片坐标范围
                auto start_x =
                    static_cast<int>(floor((world_aabb.position.x - layer_offset.x) / tile_size.x));
                auto end_x = static_cast<int>(
                    ceil((world_aabb.position.x + world_aabb.size.x - layer_offset.x - TOLERANCE) /
                         tile_size.x));
                auto start_y =
                    static_cast<int>(floor((world_aabb.position.y - layer_offset.y) / tile_size.y));
                auto end_y = static_cast<int>(
                    ceil((world_aabb.position.y + world_aabb.size.y - layer_offset.y - TOLERANCE) /
                         tile_size.y));

                // 遍历瓦片坐标范围进行检测
                for (int x = start_x; x < end_x; ++x) {
                    for (int y = start_y; y < end_y; ++y) {
                        auto tile_type = layer->getTileTypeAt({x, y});

                        // 梯子类型不加入事件，物理引擎自己处理
                        if (tile_type == engine::component::TileType::LADDER) {
                            pc->setCollidedLadder(true);
                            continue;
                        }

                        // 未来可以添加更多触发器类型的瓦片，目前只对 HAZARD 触发事件
                        if (tile_type == engine::component::TileType::HAZARD) {
                            // 检查是否已经触发过该对象和瓦片类型的组合
                            auto trigger_pair = std::make_pair(obj, tile_type);
                            if (triggered_pairs.find(trigger_pair) == triggered_pairs.end()) {
                                triggered_pairs.insert(trigger_pair);
                                tile_trigger_events_.emplace_back(obj, tile_type);
                                spdlog::trace("tile_trigger_events_中 添加了 GameObject {} "
                                              "和瓦片触发类型: {}",
                                              obj->getName(), static_cast<int>(tile_type));
                            }
                        }
                    }
                }
            }
        }
    }

    void PhysicsEngine::checkCollisionsInCell(
        const std::vector<
            std::pair<engine::object::GameObject*, engine::component::ColliderComponent*>> &objects,
        std::set<std::pair<engine::object::GameObject*, engine::object::GameObject*>>
            &checked_pairs)
    {
        for (size_t i = 0; i < objects.size(); ++i) {
            auto [obj_a, cc_a] = objects[i];

            for (size_t j = i + 1; j < objects.size(); ++j) {
                auto [obj_b, cc_b] = objects[j];

                // 确保不重复检查同一对对象
                auto pair_key = std::make_pair(std::min(obj_a, obj_b), std::max(obj_a, obj_b));

                if (checked_pairs.find(pair_key) != checked_pairs.end()) {
                    continue;
                }
                checked_pairs.insert(pair_key);

                if (!collision::checkCollision(*cc_a, *cc_b)) {
                    continue;
                }

                // 如果是可移动物体与SOLID物体碰撞，则直接处理位置变化，不用记录碰撞对
                if (obj_a->getTag() != "solid" && obj_b->getTag() == "solid") {
                    resolveSolidObjectCollisions(obj_a, obj_b);
                    continue;
                }

                if (obj_a->getTag() == "solid" && obj_b->getTag() != "solid") {
                    resolveSolidObjectCollisions(obj_b, obj_a);
                    continue;
                }

                // 记录碰撞对
                collision_pairs_.emplace_back(obj_a, obj_b);
            }
        }
    }

    void PhysicsEngine::resolveTileCollisions(engine::component::PhysicsComponent* pc,
                                              float delta_time)
    {
        engine::component::TransformComponent* tc = nullptr;
        engine::component::ColliderComponent* cc = nullptr;
        TileCollisionContext context;

        // 1. 验证输入参数
        if (!validateTileCollisionInputs(pc, tc, cc, context)) {
            return;
        }

        // 2. 计算位移
        if (!calculateTileDisplacement(pc, tc, cc, delta_time, context)) {
            return; // 位移太小，跳过处理
        }

        // 3. 遍历所有碰撞瓦片层进行碰撞检测
        for (const auto* layer : collision_tile_layers_) {
            if (!layer) continue;

            // X轴碰撞检测
            resolveXAxisTileCollision(layer, pc, context);

            // Y轴碰撞检测（使用更新后的X位置）
            resolveYAxisTileCollision(layer, pc, context);
        }

        // 4. 应用碰撞结果
        applyTileCollisionResults(tc, pc, context);
    }

    void PhysicsEngine::resolveSolidObjectCollisions(engine::object::GameObject* move_obj,
                                                     engine::object::GameObject* solid_obj)
    {
        engine::component::TransformComponent* move_tc = nullptr;
        engine::component::PhysicsComponent* move_pc = nullptr;
        engine::component::ColliderComponent* move_cc = nullptr;
        engine::component::ColliderComponent* solid_cc = nullptr;
        SolidObjectCollisionContext context;

        // 1. 验证输入参数
        if (!validateSolidObjectCollisionInputs(move_obj, solid_obj, move_tc, move_pc, move_cc,
                                                solid_cc, context)) {
            return;
        }

        // 2. 计算碰撞数据
        if (!calculateSolidObjectCollisionData(context)) {
            return; // 没有碰撞或重叠太小
        }

        // 3. 应用碰撞结果
        applySolidObjectCollisionResults(move_tc, move_pc, context);
    }

    void PhysicsEngine::applyWorldBounds(engine::component::PhysicsComponent* pc)
    {
        // 早期退出检查
        if (!pc || !world_bounds_) {
            return;
        }

        auto* obj = pc->getOwner();
        if (!obj) {
            return;
        }

        auto* cc = obj->getComponent<engine::component::ColliderComponent>();
        auto* tc = obj->getComponent<engine::component::TransformComponent>();

        // 验证组件有效性
        if (!cc || !tc || !cc->isActive()) {
            return;
        }

        const auto world_aabb = cc->getWorldAABB();

        // 验证碰撞盒有效性
        if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) {
            return;
        }

        glm::vec2 new_position = world_aabb.position;
        bool position_changed = false;

        // 预计算边界值以提高性能
        const float bounds_left = world_bounds_->position.x;
        const float bounds_top = world_bounds_->position.y;
        const float bounds_right = world_bounds_->position.x + world_bounds_->size.x;

        // 检查左边界
        if (new_position.x < bounds_left) {
            new_position.x = bounds_left;
            position_changed = true;
            if (pc->velocity_.x < 0.0f) {
                pc->velocity_.x = 0.0f;
            }
        }

        // 检查右边界
        else if (new_position.x + world_aabb.size.x > bounds_right) {
            new_position.x = bounds_right - world_aabb.size.x;
            position_changed = true;
            if (pc->velocity_.x > 0.0f) {
                pc->velocity_.x = 0.0f;
            }
        }

        // 检查上边界（只限定上边界，不限定下边界）
        if (new_position.y < bounds_top) {
            new_position.y = bounds_top;
            position_changed = true;
            if (pc->velocity_.y < 0.0f) {
                pc->velocity_.y = 0.0f;
            }
        }

        // 只有在位置确实发生变化时才更新
        if (position_changed) {
            tc->translate(new_position - world_aabb.position);
        }
    }

    bool PhysicsEngine::validateTileCollisionInputs(engine::component::PhysicsComponent* pc,
                                                    engine::component::TransformComponent*&tc,
                                                    engine::component::ColliderComponent*&cc,
                                                    TileCollisionContext &context) const
    {
        // 检查组件是否有效
        auto* obj = pc->getOwner();
        if (!obj) return false;

        tc = obj->getComponent<engine::component::TransformComponent>();
        cc = obj->getComponent<engine::component::ColliderComponent>();
        if (!tc || !cc || cc->isTrigger()) return false;

        auto world_aabb = cc->getWorldAABB(); // 使用最小包围盒进行碰撞检测（简化）
        if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) return false;

        // 初始化上下文
        context.world_aabb_position = world_aabb.position;
        context.world_aabb_size = world_aabb.size;

        return true;
    }

    bool PhysicsEngine::calculateTileDisplacement(engine::component::PhysicsComponent* pc,
                                                  engine::component::TransformComponent*&tc,
                                                  engine::component::ColliderComponent*&cc,
                                                  float delta_time,
                                                  TileCollisionContext &context) const
    {
        context.displacement = pc->velocity_ * delta_time;
        context.new_position = context.world_aabb_position + context.displacement;

        if (!cc->isActive()) {
            tc->translate(context.displacement);
            pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
            return false;
        }

        return true;
    }

    void
    PhysicsEngine::resolveXAxisTileCollision(const engine::component::TileLayerComponent* layer,
                                             engine::component::PhysicsComponent* pc,
                                             TileCollisionContext &context) const
    {
        // 早期退出：如果没有X方向位移，直接返回
        if (std::abs(context.displacement.x) < std::numeric_limits<float>::epsilon()) {
            return;
        }

        constexpr float TOLERANCE = 1.0f;            // 检查边缘时的容差值
        const auto tile_size = glm::vec2(layer->getTileSize());
        const auto inv_tile_size = 1.0f / tile_size; // 预计算倒数，避免重复除法

        // 确定检测方向和相关参数
        const bool moving_right = context.displacement.x > 0.0f;
        const float test_x = moving_right ? context.new_position.x + context.world_aabb_size.x
                                          : context.new_position.x;

        // 计算瓦片坐标
        const int tile_x = static_cast<int>(std::floor(test_x * inv_tile_size.x));
        const int tile_y_top =
            static_cast<int>(std::floor(context.world_aabb_position.y * inv_tile_size.y));
        const int tile_y_bottom = static_cast<int>(
            std::floor((context.world_aabb_position.y + context.world_aabb_size.y - TOLERANCE) *
                       inv_tile_size.y));

        // 获取瓦片类型
        const auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y_top});
        const auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});

        // 检查固体瓦片碰撞
        if (isSolidTile(tile_type_top) || isSolidTile(tile_type_bottom)) {
            handleSolidCollisionX(moving_right, tile_x, tile_size, pc, context);
            return;
        }

        // 检查斜坡瓦片碰撞（仅检查底部角点）
        handleSlopeCollisionX(moving_right, tile_x, tile_y_bottom, tile_type_bottom, tile_size, pc,
                              context);
    }

    void
    PhysicsEngine::resolveYAxisTileCollision(const engine::component::TileLayerComponent* layer,
                                             engine::component::PhysicsComponent* pc,
                                             TileCollisionContext &context) const
    {
        // 早返回：无Y位移
        if (std::abs(context.displacement.y) < std::numeric_limits<float>::epsilon()) {
            return;
        }

        constexpr float TOLERANCE = 1.0f;            // 检查边缘时的容差值
        const auto tile_size = glm::vec2(layer->getTileSize());
        const auto inv_tile_size = 1.0f / tile_size; // 预计算倒数，避免重复除法

        // 方向与测试位置
        const bool moving_down = context.displacement.y > 0.0f;
        const float test_y = moving_down ? (context.new_position.y + context.world_aabb_size.y)
                                         : context.new_position.y;

        // 计算瓦片坐标
        const int tile_y = static_cast<int>(std::floor(test_y * inv_tile_size.y));
        const int tile_x_left =
            static_cast<int>(std::floor(context.world_aabb_position.x * inv_tile_size.x));
        const int tile_x_right = static_cast<int>(
            std::floor((context.world_aabb_position.x + context.world_aabb_size.x - TOLERANCE) *
                       inv_tile_size.x));

        // 获取瓦片类型
        const auto tile_type_left = layer->getTileTypeAt({tile_x_left, tile_y});
        const auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});

        // 向上移动：仅检查天花板
        if (!moving_down) {
            if (isSolidTile(tile_type_left) || isSolidTile(tile_type_right)) {
                handleCeilingCollisionY(tile_y, tile_size, pc, context);
            }
            return;
        }

        // 向下移动：先检查地面
        if (isGroundTile(tile_type_left) || isGroundTile(tile_type_right)) {
            handleGroundCollisionY(tile_y, tile_size, pc, context);
            return;
        }

        // 向下移动：再检查梯子
        if (isLadderTile(tile_type_left) && isLadderTile(tile_type_right)) {
            const auto tile_type_up_left = layer->getTileTypeAt({tile_x_left, tile_y - 1});
            const auto tile_type_up_right = layer->getTileTypeAt({tile_x_right, tile_y - 1});
            // 如果上方不是梯子，说明刚进入或即将离开梯子，需要处理一次
            if (!isLadderTile(tile_type_up_left) && !isLadderTile(tile_type_up_right)) {
                handleLadderCollisionY(tile_y, tile_size, pc, context);
            }
            return;
        }

        // 其它情况：检查斜坡
        handleSlopeCollisionY(tile_x_left, tile_x_right, tile_y, tile_type_left, tile_type_right,
                              tile_size, pc, context);
    }

    void PhysicsEngine::applyTileCollisionResults(engine::component::TransformComponent* tc,
                                                  engine::component::PhysicsComponent* pc,
                                                  const TileCollisionContext &context) const
    {
        // 更新物体位置，并限制最大速度
        tc->translate(context.new_position - context.world_aabb_position);
        pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
    }

    bool PhysicsEngine::isSlopeTile(engine::component::TileType tile_type) const
    {
        switch (tile_type) {
            case engine::component::TileType::SLOPE_0_1:
            case engine::component::TileType::SLOPE_0_2:
            case engine::component::TileType::SLOPE_2_1:
            case engine::component::TileType::SLOPE_1_0:
            case engine::component::TileType::SLOPE_2_0:
            case engine::component::TileType::SLOPE_1_2:
                return true;
            default:
                return false;
        }
    }

    bool PhysicsEngine::isSolidTile(engine::component::TileType tile_type) const
    {
        return tile_type == engine::component::TileType::SOLID;
    }

    bool PhysicsEngine::isGroundTile(engine::component::TileType tile_type) const
    {
        return tile_type == engine::component::TileType::SOLID ||
               tile_type == engine::component::TileType::UNISOLID;
    }

    bool PhysicsEngine::isLadderTile(engine::component::TileType tile_type) const
    {
        return tile_type == engine::component::TileType::LADDER;
    }

    float PhysicsEngine::getTileHeightAtWidth(float width, engine::component::TileType tile_type,
                                              glm::vec2 tile_size) const
    {
        auto rel_x = glm::clamp(width / tile_size.x, 0.0f, 1.0f);
        switch (tile_type) {
            case engine::component::TileType::SLOPE_0_1: // 左0   右1
                return rel_x * tile_size.y;
            case engine::component::TileType::SLOPE_0_2: // 左0   右1/2
                return rel_x * tile_size.y * 0.5f;
            case engine::component::TileType::SLOPE_2_1: // 左1/2 右1
                return rel_x * tile_size.y * 0.5f + tile_size.y * 0.5f;
            case engine::component::TileType::SLOPE_1_0: // 左1   右0
                return (1.0f - rel_x) * tile_size.y;
            case engine::component::TileType::SLOPE_2_0: // 左1/2 右0
                return (1.0f - rel_x) * tile_size.y * 0.5f;
            case engine::component::TileType::SLOPE_1_2: // 左1   右1/2
                return (1.0f - rel_x) * tile_size.y * 0.5f + tile_size.y * 0.5f;
            default:
                return 0.0f; // 默认返回0，表示没有斜坡
        }
    }

    void PhysicsEngine::handleSolidCollisionX(bool moving_right, int tile_x,
                                              const glm::vec2 &tile_size,
                                              engine::component::PhysicsComponent* pc,
                                              TileCollisionContext &context) const
    {
        if (moving_right) {
            // 撞到右侧墙壁
            context.new_position.x = static_cast<float>(tile_x) * tile_size.x -
                                     context.world_aabb_size.x;
            pc->setCollidedRight(true);
        } else {
            // 撞到左侧墙壁
            context.new_position.x = static_cast<float>(tile_x + 1) * tile_size.x;
            pc->setCollidedLeft(true);
        }
        pc->velocity_.x = 0.0f;
        context.has_x_collision = true;
    }

    void PhysicsEngine::handleSlopeCollisionX(bool moving_right, int tile_x, int tile_y_bottom,
                                              engine::component::TileType tile_type_bottom,
                                              const glm::vec2 &tile_size,
                                              engine::component::PhysicsComponent* pc,
                                              TileCollisionContext &context) const
    {
        if (!isSlopeTile(tile_type_bottom)) {
            return;
        }

        const float width = moving_right
                                ? context.new_position.x + context.world_aabb_size.x -
                                      static_cast<float>(tile_x) * tile_size.x
                                : context.new_position.x - static_cast<float>(tile_x) * tile_size.x;

        const float height = getTileHeightAtWidth(width, tile_type_bottom, tile_size);
        if (height > 0.0f) {
            const float slope_surface_y = static_cast<float>(tile_y_bottom + 1) * tile_size.y -
                                          height;
            const float object_bottom_y = context.new_position.y + context.world_aabb_size.y;

            if (object_bottom_y > slope_surface_y) {
                context.new_position.y = slope_surface_y - context.world_aabb_size.y;
                pc->setCollidedBelow(true);
            }
        }
    }

    void PhysicsEngine::handleGroundCollisionY(int tile_y, const glm::vec2 &tile_size,
                                               engine::component::PhysicsComponent* pc,
                                               TileCollisionContext &context) const
    {
        // 到达地面！速度归零，y方向移动到贴着地面的位置
        context.new_position.y = static_cast<float>(tile_y) * tile_size.y -
                                 context.world_aabb_size.y;
        pc->velocity_.y = 0.0f;
        pc->setCollidedBelow(true);
        context.has_y_collision = true;
    }

    void PhysicsEngine::handleLadderCollisionY(int tile_y, const glm::vec2 &tile_size,
                                               engine::component::PhysicsComponent* pc,
                                               TileCollisionContext &context) const
    {
        // 没有重力，表明处于攀爬状态，不处理
        if (!pc->isUseGravity()) {
            return;
        }

        // 到达梯子顶部！速度归零，y方向移动到贴着梯子的位置
        context.new_position.y = static_cast<float>(tile_y) * tile_size.y -
                                 context.world_aabb_size.y;
        pc->velocity_.y = 0.0f;
        pc->setOnTopLadder(true);
        pc->setCollidedBelow(true);
        context.has_y_collision = true;
    }

    void PhysicsEngine::handleCeilingCollisionY(int tile_y, const glm::vec2 &tile_size,
                                                engine::component::PhysicsComponent* pc,
                                                TileCollisionContext &context) const
    {
        // 撞到天花板！速度归零，y方向移动到贴着天花板的位置
        context.new_position.y = static_cast<float>(tile_y + 1) * tile_size.y;
        pc->velocity_.y = 0.0f;
        pc->setCollidedAbove(true);
        context.has_y_collision = true;
    }

    void PhysicsEngine::handleSlopeCollisionY(int tile_x_left, int tile_x_right, int tile_y,
                                              engine::component::TileType tile_type_left,
                                              engine::component::TileType tile_type_right,
                                              const glm::vec2 &tile_size,
                                              engine::component::PhysicsComponent* pc,
                                              TileCollisionContext &context) const
    {
        // 检测斜坡瓦片（下方两个角点都要检测）
        const float width_left = context.world_aabb_position.x -
                                 static_cast<float>(tile_x_left) * tile_size.x;
        const float width_right = context.world_aabb_position.x + context.world_aabb_size.x -
                                  static_cast<float>(tile_x_right) * tile_size.x;

        const float height_left = getTileHeightAtWidth(width_left, tile_type_left, tile_size);
        const float height_right = getTileHeightAtWidth(width_right, tile_type_right, tile_size);
        const float height = std::max(height_left, height_right); // 找到两个角点的最高点进行检测

        if (height > 0.0f) {                                      // 说明至少有一个角点处于斜坡瓦片
            const float slope_surface_y = static_cast<float>(tile_y + 1) * tile_size.y - height;
            const float object_bottom_y = context.new_position.y + context.world_aabb_size.y;

            if (object_bottom_y > slope_surface_y) {
                context.new_position.y = slope_surface_y - context.world_aabb_size.y;
                pc->velocity_.y = 0.0f; // 只有向下运动时才需要让 y 速度归零
                pc->setCollidedBelow(true);
                context.has_y_collision = true;
            }
        }
    }

    bool PhysicsEngine::validateSolidObjectCollisionInputs(
        engine::object::GameObject* move_obj, engine::object::GameObject* solid_obj,
        engine::component::TransformComponent*&move_tc,
        engine::component::PhysicsComponent*&move_pc, engine::component::ColliderComponent*&move_cc,
        engine::component::ColliderComponent*&solid_cc, SolidObjectCollisionContext &context) const
    {
        // 验证移动对象
        if (!move_obj) return false;

        move_tc = move_obj->getComponent<engine::component::TransformComponent>();
        move_pc = move_obj->getComponent<engine::component::PhysicsComponent>();
        move_cc = move_obj->getComponent<engine::component::ColliderComponent>();

        if (!move_tc || !move_pc || !move_cc || !move_cc->isActive() || move_cc->isTrigger()) {
            return false;
        }

        // 验证固体对象
        if (!solid_obj) return false;

        solid_cc = solid_obj->getComponent<engine::component::ColliderComponent>();

        if (!solid_cc || !solid_cc->isActive() || solid_cc->isTrigger()) {
            return false;
        }

        // 获取AABB信息并验证有效性
        const auto move_aabb = move_cc->getWorldAABB();
        const auto solid_aabb = solid_cc->getWorldAABB();

        if (move_aabb.size.x <= 0.0f || move_aabb.size.y <= 0.0f || solid_aabb.size.x <= 0.0f ||
            solid_aabb.size.y <= 0.0f) {
            return false;
        }

        // 初始化上下文
        context.move_aabb_position = move_aabb.position;
        context.move_aabb_size = move_aabb.size;
        context.solid_aabb_position = solid_aabb.position;
        context.solid_aabb_size = solid_aabb.size;

        return true;
    }

    bool
    PhysicsEngine::calculateSolidObjectCollisionData(SolidObjectCollisionContext &context) const
    {
        // 计算中心点
        context.move_center = context.move_aabb_position + context.move_aabb_size / 2.0f;
        context.solid_center = context.solid_aabb_position + context.solid_aabb_size / 2.0f;

        // 计算重叠部分
        context.overlap =
            glm::vec2(context.move_aabb_size / 2.0f + context.solid_aabb_size / 2.0f) -
            glm::abs(context.move_center - context.solid_center);

        // 检查是否有有效碰撞
        constexpr float MIN_OVERLAP = 0.01f; // 使用更小的阈值以提高精度
        if (context.overlap.x < MIN_OVERLAP && context.overlap.y < MIN_OVERLAP) {
            context.has_collision = false;
            return false;
        }

        context.has_collision = true;
        return true;
    }

    void PhysicsEngine::applySolidObjectCollisionResults(
        engine::component::TransformComponent* move_tc,
        engine::component::PhysicsComponent* move_pc,
        const SolidObjectCollisionContext &context) const
    {
        if (!context.has_collision) return;

        // 使用EPSILON避免浮点精度问题
        constexpr float EPSILON = 0.01f;

        // 选择最小重叠方向进行分离
        glm::vec2 translation(0.0f);
        if (context.overlap.x < context.overlap.y) {
            // X轴分离
            float dx = context.overlap.x + EPSILON;
            if (context.move_center.x < context.solid_center.x) {
                // 移动物体在左边，向左推出
                translation.x = -dx;
                // 只有当速度朝向碰撞方向时才清零
                if (move_pc->velocity_.x > 0.0f) {
                    move_pc->velocity_.x = 0.0f;
                    move_pc->setCollidedRight(true);
                }
            } else {
                // 移动物体在右边，向右推出
                translation.x = dx;
                if (move_pc->velocity_.x < 0.0f) {
                    move_pc->velocity_.x = 0.0f;
                    move_pc->setCollidedLeft(true);
                }
            }
        } else {
            // Y轴分离
            float dy = context.overlap.y + EPSILON;
            if (context.move_center.y < context.solid_center.y) {
                // 移动物体在上面，向上推出
                translation.y = -dy;
                if (move_pc->velocity_.y > 0.0f) {
                    move_pc->velocity_.y = 0.0f;
                    move_pc->setCollidedBelow(true);
                }
            } else {
                // 移动物体在下面，向下推出
                translation.y = dy;
                if (move_pc->velocity_.y < 0.0f) {
                    move_pc->velocity_.y = 0.0f;
                    move_pc->setCollidedAbove(true);
                }
            }
        }

        move_tc->translate(translation);

        // 应用速度限制（与瓦片碰撞保持一致）
        move_pc->velocity_.x = std::clamp(move_pc->velocity_.x, -max_speed_, max_speed_);
        move_pc->velocity_.y = std::clamp(move_pc->velocity_.y, -max_speed_, max_speed_);
    }
} // namespace engine::physics
