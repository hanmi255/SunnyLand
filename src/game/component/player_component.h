#pragma once
#include "../../engine/component/component.h"
#include "state/player_state.h"
#include <memory>

namespace engine::component {
    class AnimationComponent;
    class HealthComponent;
    class PhysicsComponent;
    class SpriteComponent;
    class TransformComponent;
} // namespace engine::component

namespace engine::input {
    class InputManager;
} // namespace engine::input

namespace game::component::state {
    class PlayerState;
} // namespace game::component::state

namespace game::component {

    /**
     * @brief 处理玩家输入、状态和控制 GameObject 移动的组件。
     *        使用状态模式管理 Idle, Walk, Jump, Fall 等状态。
     */
    class PlayerComponent final : public engine::component::Component {
        friend class engine::object::GameObject;

    private:
        engine::component::AnimationComponent* animation_component_ = nullptr; ///< @brief 动画组件
        engine::component::HealthComponent* health_component_ = nullptr;       ///< @brief 生命组件
        engine::component::PhysicsComponent* physics_component_ = nullptr;     ///< @brief 物理组件
        engine::component::SpriteComponent* sprite_component_ = nullptr;       ///< @brief 精灵组件
        engine::component::TransformComponent* transform_component_ = nullptr; ///< @brief 变换组件

        std::unique_ptr<state::PlayerState> current_state_;                    ///< @brief 当前状态
        bool is_dead_ = false;                                                 ///< @brief 是否死亡

        // --- 移动相关参数 ---
        float move_force_ = 200.0f;     ///< @brief 水平移动力
        float max_speed_ = 120.0f;      ///< @brief 最大移动速度 (像素/秒)
        float friction_factor_ = 0.85f; ///< @brief 摩擦系数 (Idle时缓冲效果，每帧乘以此系数)
        float jump_force_ = 350.0f;     ///< @brief 跳跃力 (按下"jump"键给的瞬间向上的力)

        // --- 状态相关参数 ---
        float stunned_duration_ = 0.4f; ///< @brief 击退硬直效果持续时间

    public:
        PlayerComponent() = default;
        ~PlayerComponent() override = default;

        // 禁止拷贝和移动语义
        PlayerComponent(const PlayerComponent &) = delete;
        PlayerComponent &operator=(const PlayerComponent &) = delete;
        PlayerComponent(PlayerComponent &&) = delete;
        PlayerComponent &operator=(PlayerComponent &&) = delete;

        bool takeDamage(int damage_amount);

        // --- getters ---
        engine::component::AnimationComponent* getAnimationComponent() const
        {
            return animation_component_;
        }
        engine::component::PhysicsComponent* getPhysicsComponent() const
        {
            return physics_component_;
        }
        engine::component::SpriteComponent* getSpriteComponent() const { return sprite_component_; }
        engine::component::TransformComponent* getTransformComponent() const
        {
            return transform_component_;
        }

        bool isDead() const { return is_dead_; }
        float getMoveForce() const { return move_force_; }
        float getMaxSpeed() const { return max_speed_; }
        float getFrictionFactor() const { return friction_factor_; }
        float getJumpForce() const { return jump_force_; }
        float getStunnedDuration() const { return stunned_duration_; }

        // --- setters ---
        void setIsDead(bool is_dead) { is_dead_ = is_dead; }
        void setMoveForce(float move_force) { move_force_ = move_force; }
        void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
        void setFrictionFactor(float friction_factor) { friction_factor_ = friction_factor; }
        void setJumpForce(float jump_force) { jump_force_ = jump_force; }
        void setStunnedDuration(float duration) { stunned_duration_ = duration; }

        void setState(std::unique_ptr<state::PlayerState> new_state);

    private:
        // 核心逻辑
        void init() override;
        void handleInput(engine::core::Context &context) override;
        void update(float delta_time, engine::core::Context &context) override;
    };
} // namespace game::component
