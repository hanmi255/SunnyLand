#include "climb_state.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "fall_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include <glm/common.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace game::component::player_state {

    void ClimbState::enter()
    {
        spdlog::debug("玩家进入攀爬状态。");
        playAnimation("climb");
        if (auto* physics = player_component_->getPhysicsComponent(); physics) {
            physics->setUseGravity(false);
        }
    }

    void ClimbState::exit()
    {
        spdlog::debug("玩家退出攀爬状态。");
        if (auto* physics = player_component_->getPhysicsComponent(); physics) {
            physics->setUseGravity(true);
        }
    }

    std::unique_ptr<PlayerState> ClimbState::handleInput(engine::core::Context &context)
    {
        const auto& input_manager = context.getInputManager();
        auto* physics_component = player_component_->getPhysicsComponent();
        auto* animation_component = player_component_->getAnimationComponent();

        // --- 攀爬状态下，按键则移动，否则保持静止
        auto is_up = input_manager.isActionHeldDown("move_up");
        auto is_down = input_manager.isActionHeldDown("move_down");
        auto is_left = input_manager.isActionHeldDown("move_left");
        auto is_right = input_manager.isActionHeldDown("move_right");
        auto speed = player_component_->getClimbSpeed();

        physics_component->velocity_.x = is_left ? -speed : is_right ? speed : 0.0F;
        physics_component->velocity_.y = is_up ? -speed : is_down ? speed : 0.0F;

        (is_up || is_down || is_left || is_right) ? animation_component->resumeAnimation()
                                                  : animation_component->stopAnimation();

        if (input_manager.isActionJustPressed("jump")) {
            return std::make_unique<JumpState>(player_component_);
        }

        return nullptr;
    }

    std::unique_ptr<PlayerState> ClimbState::update(float  /*unused*/, engine::core::Context & /*unused*/)
    {
        auto* physics_component = player_component_->getPhysicsComponent();
        // 如果着地，则切换到 IdleState
        if (physics_component->hasCollidedBelow()) {
            return std::make_unique<IdleState>(player_component_);
        }
        // 离开梯子，则切换到 FallState
        if (!physics_component->hasCollidedLadder()) {
            return std::make_unique<FallState>(player_component_);
        }

        return nullptr;
    }
} // namespace game::component::player_state