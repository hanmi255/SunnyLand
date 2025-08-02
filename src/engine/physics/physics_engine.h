#pragma once
#include <glm/vec2.hpp>
#include <vector>

namespace engine::component {
    class PhysicsComponent;
} // namespace engine::component

namespace engine::physics {

    /**
     * @brief 负责管理和模拟物理行为及碰撞检测。
     */
    class PhysicsEngine {
    private:
        std::vector<engine::component::PhysicsComponent*>
            components_;               ///< @brief 注册的物理组件容器，非拥有指针
        glm::vec2 gravity_ = {0.0f,
                              980.0f}; ///< @brief 默认重力值 (像素/秒^2, 相当于100像素对应现实1m)
        float max_speed_ = 500.0f;

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

        // --- getters ---
        const glm::vec2 &getGravity() const { return gravity_; }
        float getMaxSpeed() const { return max_speed_; }

        // --- setters ---
        void setGravity(const glm::vec2 &gravity) { gravity_ = gravity; }
        void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
    };
} // namespace engine::physics
