#include "jump_state.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "climb_state.h"
#include "fall_state.h"
#include <algorithm>
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component::player_state {

    void JumpState::enter()
    {
        // 向上跳跃，并播放跳跃音效
        playAnimation("jump");
        auto* physics_component = player_component_->getPhysicsComponent();
        physics_component->velocity_.y = -player_component_->getJumpVelocity(); // 向上跳跃
        if (auto* audio_component = player_component_->getAudioComponent(); audio_component) {
            audio_component->playSound("jump");
        }
        spdlog::debug("PlayerComponent 进入 JumpState，设置初始垂直速度为: {}",
                      physics_component->velocity_.y);
    }

    void JumpState::exit() {}

    std::unique_ptr<PlayerState> JumpState::handleInput(engine::core::Context &context)
    {
        const auto& input_manager = context.getInputManager();
        auto* physics_component = player_component_->getPhysicsComponent();
        auto* sprite_component = player_component_->getSpriteComponent();

        // 接触梯子并且按移动键
        if (physics_component->hasCollidedLadder() &&
            (input_manager.isActionHeldDown("move_up") ||
             input_manager.isActionHeldDown("move_down"))) {
            return std::make_unique<ClimbState>(player_component_);
        }

        // 跳跃状态下可以左右移动
        if (input_manager.isActionHeldDown("move_left")) {
            physics_component->velocity_.x = std::min(physics_component->velocity_.x, 0.0F);
            physics_component->addForce({-player_component_->getMoveForce(), 0.0F});
            sprite_component->setFlipped(true);
        } else if (input_manager.isActionHeldDown("move_right")) {
            physics_component->velocity_.x = std::max(physics_component->velocity_.x, 0.0F);
            physics_component->addForce({player_component_->getMoveForce(), 0.0F});
            sprite_component->setFlipped(false);
        }
        return nullptr;
    }

    std::unique_ptr<PlayerState> JumpState::update(float /*unused*/, engine::core::Context & /*unused*/)
    {
        // 限制最大速度(水平方向)
        auto* physics_component = player_component_->getPhysicsComponent();
        auto max_speed = player_component_->getMaxSpeed();
        physics_component->velocity_.x =
            glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

        // 如果速度为正，切换到 FallState
        if (physics_component->velocity_.y >= 0.0F) {
            return std::make_unique<FallState>(player_component_);
        }

        return nullptr;
    }

} // namespace game::component::player_state
