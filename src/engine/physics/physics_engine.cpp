#include "physics_engine.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../object/game_object.h"
#include "collision.h"
#include <algorithm>
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::physics {

    int PhysicsEngine::SpatialGrid::getGridKey(float x, float y) const
    {
        int grid_x = static_cast<int>(x / cell_size);
        int grid_y = static_cast<int>(y / cell_size);
        return grid_x * 10000 + grid_y; // 简单的hash
    }

    void PhysicsEngine::SpatialGrid::clear()
    {
        grid.clear();
    }

    void PhysicsEngine::SpatialGrid::insert(engine::object::GameObject* obj,
                                            engine::component::ColliderComponent* cc)
    {
        auto transform = obj->getComponent<engine::component::TransformComponent>();
        if (!transform) return;

        auto pos = transform->getPosition();
        int key = getGridKey(pos.x, pos.y);
        grid[key].emplace_back(obj, cc);
    }

    std::vector<int> PhysicsEngine::SpatialGrid::getNearbyKeys(float x, float y) const
    {
        std::vector<int> keys;
        keys.reserve(9); // 最多9个相邻格子

        int center_x = static_cast<int>(x / cell_size);
        int center_y = static_cast<int>(y / cell_size);

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                keys.push_back((center_x + dx) * 10000 + (center_y + dy));
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

            // 更新位置：S += v * dt
            auto* tc = pc->getTransformComponent();
            if (tc) {
                tc->translate(pc->velocity_ * delta_time);
            }

            // 限制最大速度：v = min(v, max_speed)
            pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
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

        // 第二步：只检查同一网格或相邻网格的对象
        std::set<std::pair<engine::object::GameObject*, engine::object::GameObject*>>
            checked_pairs; // 避免重复检查

        for (const auto &[grid_key, objects] : spatial_grid_.grid) {
            // 检查同一网格内的对象
            checkCollisionsInCell(objects, checked_pairs);

            // 检查相邻网格（可选，取决于网格大小和对象大小的关系）
            // 这里简化处理，只检查同一网格内的碰撞
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

                if (collision::checkCollision(*cc_a, *cc_b)) {
                    collision_pairs_.emplace_back(obj_a, obj_b);
                }
            }
        }
    }

} // namespace engine::physics
