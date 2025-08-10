#include "player_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/audio_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/object/game_object.h"
#include "player_state/dead_state.h"
#include "player_state/hurt_state.h"
#include "player_state/idle_state.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace game::component {

    void PlayerComponent::init()
    {
        if (!owner_) {
            spdlog::error("PlayerComponent 在初始化前未设置 owner_。");
            return;
        }

        // 获取必要的组件
        transform_component_ = owner_->getComponent<engine::component::TransformComponent>();
        physics_component_ = owner_->getComponent<engine::component::PhysicsComponent>();
        sprite_component_ = owner_->getComponent<engine::component::SpriteComponent>();
        animation_component_ = owner_->getComponent<engine::component::AnimationComponent>();
        health_component_ = owner_->getComponent<engine::component::HealthComponent>();
        audio_component_ = owner_->getComponent<engine::component::AudioComponent>();

        // 检查必要组件是否存在
        if (!transform_component_ || !physics_component_ || !sprite_component_ ||
            !animation_component_ || !health_component_ || !audio_component_) {
            spdlog::error("Player 对象缺少必要组件！");
        }

        // 初始化状态机
        current_state_ = std::make_unique<player_state::IdleState>(this);
        if (current_state_) {
            setState(std::move(current_state_));
        } else {
            spdlog::error("初始化玩家状态失败（make_unique 返回空指针）！");
        }
        spdlog::debug("PlayerComponent 初始化完成。");
    }

    void PlayerComponent::setState(std::unique_ptr<player_state::PlayerState> new_state)
    {
        if (!new_state) {
            spdlog::warn("尝试设置空的玩家状态！");
            return;
        }
        if (current_state_) {
            current_state_->exit();
        }

        spdlog::debug("玩家组件正在切换到状态: {}", new_state->getStateName());
        current_state_ = std::move(new_state);
        current_state_->enter();
    }

    bool PlayerComponent::isOnGround() const
    {
        return coyote_timer_ <= coyote_time_ || physics_component_->hasCollidedBelow();
    }

    void PlayerComponent::handleInput(engine::core::Context &context)
    {
        if (!current_state_) return;

        auto next_state = current_state_->handleInput(context);
        if (next_state) {
            setState(std::move(next_state));
        }
    }

    void PlayerComponent::update(float delta_time, engine::core::Context &context)
    {
        if (!current_state_) return;

        // 更新 Coyote Timer：着地时重置，离地时计时
        if (physics_component_->hasCollidedBelow()) {
            coyote_timer_ = 0.0f;
        } else {
            coyote_timer_ += delta_time;
        }

        // 处理无敌状态的闪烁效果
        const bool is_invincible = health_component_->isInvincible();
        if (is_invincible) {
            flash_timer_ += delta_time;

            // 使用 fmod 确保计时器在 [0, 2*flash_interval_) 范围内循环
            const float flash_cycle = 2.0f * flash_interval_;
            flash_timer_ = std::fmod(flash_timer_, flash_cycle);

            // 前半周期可见，后半周期不可见
            sprite_component_->setVisible(flash_timer_ < flash_interval_);
        }
        // 非无敌状态时确保精灵可见
        else if (!sprite_component_->isVisible()) {
            sprite_component_->setVisible(true);
        }

        // 更新状态机
        if (auto next_state = current_state_->update(delta_time, context)) {
            setState(std::move(next_state));
        }
    }

    bool PlayerComponent::takeDamage(int damage_amount)
    {
        if (is_dead_ || !health_component_ || damage_amount <= 0) {
            spdlog::warn("玩家已死亡或缺少必要组件，并未造成伤害。");
            return false;
        }

        bool success = health_component_->takeDamage(damage_amount);
        if (!success) return false;

        if (health_component_->isAlive()) {
            spdlog::debug("玩家受到了 {} 点伤害，当前生命值: {}/{}。", damage_amount,
                          health_component_->getCurrentHealth(), health_component_->getMaxHealth());
            setState(std::make_unique<player_state::HurtState>(this));
        } else {
            spdlog::debug("玩家死亡。");
            is_dead_ = true;
            setState(std::make_unique<player_state::DeadState>(this));
        }
        return true;
    }

} // namespace game::component
