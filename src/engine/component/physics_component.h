#pragma once
#include "component.h"
#include <glm/vec2.hpp>

namespace engine::physics {
    class PhysicsEngine;
} // namespace engine::physics

namespace engine::component {
    class TransformComponent;

    class PhysicsComponent final : public Component {
        friend class engine::object::GameObject;

    public:
        glm::vec2 velocity_ = {0.0f, 0.0f}; ///<@brief 物体的速度

    private:
        engine::physics::PhysicsEngine* physics_engine_ = nullptr; ///<@brief 物理引擎
        TransformComponent* transform_component_ = nullptr;        ///<@brief 物体的变换组件

        glm::vec2 force_ = {0.0f, 0.0f};                           ///<@brief 物体的力
        float mass_ = 1.0f;                                        ///<@brief 物体的质量 默认为1.0f
        bool use_gravity_ = true;                                  ///< @brief 物体是否受重力影响
        bool enabled_ = true;                                      ///<@brief 是否启用组件

    public:
        /**
         * @brief 构造函数
         *
         * @param physics_engine 指向PhysicsEngine的指针，不能为nullptr
         * @param use_gravity 物体是否受重力影响，默认true
         * @param mass 物体质量，默认1.0
         */
        PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity = true,
                         float mass = 1.0f);
        ~PhysicsComponent() override = default;

        // 禁用拷贝和移动语义
        PhysicsComponent(const PhysicsComponent &) = delete;
        PhysicsComponent &operator=(const PhysicsComponent &) = delete;
        PhysicsComponent(PhysicsComponent &&) = delete;
        PhysicsComponent &operator=(PhysicsComponent &&) = delete;

        // PhysicsEngine 使用的物理方法
        void addForce(const glm::vec2 &force)
        {
            if (enabled_) force_ += force;
        }
        void clearForce() { force_ = {0.0f, 0.0f}; }
        const glm::vec2 &getForce() const { return force_; }
        float getMass() const { return mass_; }
        bool isEnabled() const { return enabled_; }
        bool isUseGravity() const { return use_gravity_; }

        // 其他方法
        // --- getters ---
        const glm::vec2 &getVelocity() const { return velocity_; }
        TransformComponent* getTransformComponent() const { return transform_component_; }

        // --- setters ---
        void setEnabled(bool enabled) { enabled_ = enabled; }
        void setMass(float mass) { mass_ = (mass >= 0.0f) ? mass : 1.0f; }
        void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }
        void setVelocity(const glm::vec2 &velocity) { velocity_ = velocity; }

    private:
        // 核心逻辑
        void init() override;
        void update(float, engine::core::Context &) override {}
        void clean() override;
    };
} // namespace engine::component
