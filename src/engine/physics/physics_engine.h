#pragma once
#include <glm/vec2.hpp>
#include <set>     // 用于 std::set
#include <unordered_map>
#include <utility> // 用于 std::pair
#include <vector>

namespace engine::component {
    class ColliderComponent;
    class PhysicsComponent;
    class TransformComponent;
} // namespace engine::component

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace engine::physics {

    /**
     * @brief 负责管理和模拟物理行为及碰撞检测。
     */
    class PhysicsEngine {
    private:
        // 简单的网格空间分割
        struct SpatialGrid {
            float cell_size;
            std::unordered_map<int, std::vector<std::pair<engine::object::GameObject*,
                                                          engine::component::ColliderComponent*>>>
                grid;

            SpatialGrid(float size = 100.0f) : cell_size(size) {}

            int getGridKey(float x, float y) const;
            void clear();
            void insert(engine::object::GameObject* obj, engine::component::ColliderComponent* cc);
            std::vector<int> getNearbyKeys(float x, float y) const;
        };

    private:
        std::vector<engine::component::PhysicsComponent*>
            components_;               ///< @brief 注册的物理组件容器，非拥有指针
        glm::vec2 gravity_ = {0.0f,
                              980.0f}; ///< @brief 默认重力值 (像素/秒^2, 相当于100像素对应现实1m)
        float max_speed_ = 500.0f;     ///<< @brief 最大速度

        std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>>
            collision_pairs_;      /// < @brief 物体碰撞对（每次 update 都会清空）
        SpatialGrid spatial_grid_; /// < @brief 空间网格

    public:
        PhysicsEngine() = default;

        // 禁止拷贝和移动语义
        PhysicsEngine(const PhysicsEngine &) = delete;
        PhysicsEngine &operator=(const PhysicsEngine &) = delete;
        PhysicsEngine(PhysicsEngine &&) = delete;
        PhysicsEngine &operator=(PhysicsEngine &&) = delete;

        void registerComponent(engine::component::PhysicsComponent* component);
        void unregisterComponent(engine::component::PhysicsComponent* component);

        void update(float delta_time);
        void checkObjectCollisions();

        // --- getters ---
        const glm::vec2 &getGravity() const { return gravity_; }
        float getMaxSpeed() const { return max_speed_; }
        const std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>> &
        getCollisionPairs() const
        {
            return collision_pairs_;
        }

        // --- setters ---
        void setGravity(const glm::vec2 &gravity) { gravity_ = gravity; }
        void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }

    private:
        void checkCollisionsInCell(
            const std::vector<std::pair<engine::object::GameObject*,
                                        engine::component::ColliderComponent*>> &objects,
            std::set<std::pair<engine::object::GameObject*, engine::object::GameObject*>>
                &checked_pairs);
    };
} // namespace engine::physics
