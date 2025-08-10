#include "hurt_state.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../player_component.h"
#include "fall_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include <glm/common.hpp>

namespace game::component::player_state {

    void HurtState::enter()
    {
        playAnimation("hurt");

        // 造成击退效果，并播放受伤音效
        auto* physics_component = player_component_->getPhysicsComponent();
        auto* sprite_component = player_component_->getSpriteComponent();
        auto knockback_velocity = glm::vec2(-100.0f, -150.0f); // 默认左上方击退效果
        // 根据当前精灵的朝向状态决定是否改成右上方
        if (sprite_component->isFlipped()) {
            knockback_velocity.x = -knockback_velocity.x;  // 变成向右
        }
        physics_component->velocity_ = knockback_velocity; // 设置击退速度
        if (auto* audio_component = player_component_->getAudioComponent(); audio_component) {
            audio_component->playSound("hurt");
        }
    }

    void HurtState::exit() {}

    std::unique_ptr<PlayerState> HurtState::handleInput(engine::core::Context &)
    {
        return nullptr;
    }

    std::unique_ptr<PlayerState> HurtState::update(float delta_time, engine::core::Context &)
    {
        stunned_timer_ += delta_time;

        auto* physics_component = player_component_->getPhysicsComponent();

        // 如果角色已经落地
        if (physics_component->hasCollidedBelow()) {
            // 水平速度接近0时进入空闲状态，否则进入行走状态
            const float kMinVelocityForWalk = 1.0f;
            if (glm::abs(physics_component->velocity_.x) < kMinVelocityForWalk) {
                return std::make_unique<IdleState>(player_component_);
            } else {
                return std::make_unique<WalkState>(player_component_);
            }
        }

        // 硬直时间结束后进入下落状态
        if (stunned_timer_ > player_component_->getStunnedDuration()) {
            stunned_timer_ = 0.0f;
            return std::make_unique<FallState>(player_component_);
        }

        return nullptr;
    }

} // namespace game::component::player_state
