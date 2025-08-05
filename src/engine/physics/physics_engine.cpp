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
        // 每帧开始时先清空碰撞对列表
        collision_pairs_.clear();

        // 遍历所有注册的物理组件
        for (auto* pc : components_) {
            if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
                continue;
            }

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
        }

        // 处理对象间碰撞
        checkObjectCollisions();
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
        if (!calculateTileDisplacement(pc, delta_time, context)) {
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

    bool PhysicsEngine::validateTileCollisionInputs(engine::component::PhysicsComponent* pc,
                                                    engine::component::TransformComponent*&tc,
                                                    engine::component::ColliderComponent*&cc,
                                                    TileCollisionContext &context) const
    {
        auto* obj = pc->getOwner();
        if (!obj) return false;

        tc = obj->getComponent<engine::component::TransformComponent>();
        cc = obj->getComponent<engine::component::ColliderComponent>();

        if (!tc || !cc || !cc->isActive() || cc->isTrigger()) {
            return false;
        }

        const auto world_aabb = cc->getWorldAABB();
        if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) {
            return false;
        }

        // 初始化上下文
        context.world_aabb_position = world_aabb.position;
        context.world_aabb_size = world_aabb.size;

        return true;
    }

    bool PhysicsEngine::calculateTileDisplacement(engine::component::PhysicsComponent* pc,
                                                  float delta_time,
                                                  TileCollisionContext &context) const
    {
        context.displacement = pc->velocity_ * delta_time;

        // 早期退出：位移太小
        constexpr float MIN_DISPLACEMENT_SQ = 0.001f * 0.001f; // 最小位移平方
        if (glm::dot(context.displacement, context.displacement) < MIN_DISPLACEMENT_SQ) {
            return false;
        }

        context.new_position = context.world_aabb_position + context.displacement;
        return true;
    }

    void
    PhysicsEngine::resolveXAxisTileCollision(const engine::component::TileLayerComponent* layer,
                                             engine::component::PhysicsComponent* pc,
                                             TileCollisionContext &context) const
    {
        // 用于避免浮点数精度问题的小数值，防止在边界检测时由于精度误差产生错误碰撞
        constexpr float EPSILON = 0.01f;

        if (std::abs(context.displacement.x) <= EPSILON) {
            return;
        }

        const glm::vec2 &tile_size = layer->getTileSize();
        const glm::vec2 inv_tile_size(1.0f / tile_size.x, 1.0f / tile_size.y);

        // 计算碰撞边缘和目标瓦片
        const float edge_x = (context.displacement.x > 0)
                                 ? context.new_position.x + context.world_aabb_size.x
                                 : context.new_position.x;
        const int tile_x = static_cast<int>(std::floor(edge_x * inv_tile_size.x));

        // 计算Y方向瓦片范围
        auto [tile_y_min, tile_y_max] = calculateTileRange(
            context.world_aabb_position.y, context.world_aabb_size.y, inv_tile_size, EPSILON);

        // 检查垂直范围内的瓦片碰撞
        bool has_collision = checkTileCollisionInRange(layer, tile_x, tile_y_min, tile_y_max, true);

        if (has_collision) {
            if (context.displacement.x > 0) {
                context.new_position.x = tile_x * tile_size.x - context.world_aabb_size.x - EPSILON;
            } else {
                context.new_position.x = (tile_x + 1) * tile_size.x + EPSILON;
            }
            pc->velocity_.x = 0.0f;
            context.has_x_collision = true;
        }
    }

    void
    PhysicsEngine::resolveYAxisTileCollision(const engine::component::TileLayerComponent* layer,
                                             engine::component::PhysicsComponent* pc,
                                             TileCollisionContext &context) const
    {
        // 用于避免浮点数精度问题的小数值，防止在边界检测时由于精度误差产生错误碰撞
        constexpr float EPSILON = 0.01f;

        if (std::abs(context.displacement.y) <= EPSILON) {
            return;
        }

        const glm::vec2 &tile_size = layer->getTileSize();
        const glm::vec2 inv_tile_size(1.0f / tile_size.x, 1.0f / tile_size.y);

        // 计算碰撞边缘和目标瓦片
        const float edge_y = (context.displacement.y > 0)
                                 ? context.new_position.y + context.world_aabb_size.y
                                 : context.new_position.y;
        const int tile_y = static_cast<int>(std::floor(edge_y * inv_tile_size.y));

        // 使用更新后的X位置计算范围
        auto [tile_x_min, tile_x_max] = calculateTileRange(
            context.new_position.x, context.world_aabb_size.x, inv_tile_size, EPSILON);

        // 检查水平范围内的瓦片碰撞
        bool has_collision =
            checkTileCollisionInRange(layer, tile_y, tile_x_min, tile_x_max, false);

        if (has_collision) {
            if (context.displacement.y > 0) {
                context.new_position.y = tile_y * tile_size.y - context.world_aabb_size.y - EPSILON;
            } else {
                context.new_position.y = (tile_y + 1) * tile_size.y + EPSILON;
            }
            pc->velocity_.y = 0.0f;
            context.has_y_collision = true;
        }
    }

    void PhysicsEngine::applyTileCollisionResults(engine::component::TransformComponent* tc,
                                                  engine::component::PhysicsComponent* pc,
                                                  const TileCollisionContext &context) const
    {
        // 应用位置更新
        tc->translate(context.new_position - context.world_aabb_position);

        // 应用速度限制
        pc->velocity_.x = std::clamp(pc->velocity_.x, -max_speed_, max_speed_);
        pc->velocity_.y = std::clamp(pc->velocity_.y, -max_speed_, max_speed_);
    }

    bool
    PhysicsEngine::checkTileCollisionInRange(const engine::component::TileLayerComponent* layer,
                                             int tile_coord, int range_min, int range_max,
                                             bool is_x_axis) const
    {
        for (int i = range_min; i <= range_max; ++i) {
            const glm::ivec2 tile_pos = is_x_axis ? glm::ivec2(tile_coord, i)
                                                  : glm::ivec2(i, tile_coord);

            if (layer->getTileTypeAt(tile_pos) == engine::component::TileType::SOLID) {
                return true;
            }
        }
        return false;
    }

    std::pair<int, int> PhysicsEngine::calculateTileRange(float position, float size,
                                                          const glm::vec2 &inv_tile_size,
                                                          float epsilon) const
    {
        // 计算瓦片范围
        const float inv_size = (inv_tile_size.x != 0) ? inv_tile_size.x : inv_tile_size.y;

        int range_min = static_cast<int>(std::floor(position * inv_size));
        int range_max = static_cast<int>(std::floor((position + size - epsilon) * inv_size));

        return {range_min, range_max};
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
        if (context.overlap.x < context.overlap.y) {
            // X轴分离
            if (context.move_center.x < context.solid_center.x) {
                // 移动物体在左边，向左推出
                move_tc->translate(glm::vec2(-context.overlap.x - EPSILON, 0.0f));
                // 只有当速度朝向碰撞方向时才清零
                if (move_pc->velocity_.x > 0.0f) {
                    move_pc->velocity_.x = 0.0f;
                }
            } else {
                // 移动物体在右边，向右推出
                move_tc->translate(glm::vec2(context.overlap.x + EPSILON, 0.0f));
                if (move_pc->velocity_.x < 0.0f) {
                    move_pc->velocity_.x = 0.0f;
                }
            }
        } else {
            // Y轴分离
            if (context.move_center.y < context.solid_center.y) {
                // 移动物体在上面，向上推出
                move_tc->translate(glm::vec2(0.0f, -context.overlap.y - EPSILON));
                if (move_pc->velocity_.y > 0.0f) {
                    move_pc->velocity_.y = 0.0f;
                }
            } else {
                // 移动物体在下面，向下推出
                move_tc->translate(glm::vec2(0.0f, context.overlap.y + EPSILON));
                if (move_pc->velocity_.y < 0.0f) {
                    move_pc->velocity_.y = 0.0f;
                }
            }
        }

        // 应用速度限制（与瓦片碰撞保持一致）
        move_pc->velocity_.x = std::clamp(move_pc->velocity_.x, -max_speed_, max_speed_);
        move_pc->velocity_.y = std::clamp(move_pc->velocity_.y, -max_speed_, max_speed_);
    }

} // namespace engine::physics
