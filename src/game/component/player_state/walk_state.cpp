#include "walk_state.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../../../engine/object/game_object.h"
#include "../player_component.h"
#include "climb_state.h"
#include "fall_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include <algorithm>
#include <glm/common.hpp>

namespace game::component::player_state {

    void WalkState::enter()
    {
        playAnimation("walk");
    }

    void WalkState::exit() {}

    std::unique_ptr<PlayerState> WalkState::handleInput(engine::core::Context &context)
    {
        const auto& input_manager = context.getInputManager();
        auto* physics_component = player_component_->getPhysicsComponent();
        auto* sprite_component = player_component_->getSpriteComponent();

        // 接触梯子并且按移动键
        if (physics_component->hasCollidedLadder() && input_manager.isActionHeldDown("move_up")) {
            return std::make_unique<ClimbState>(player_component_);
        }

        // 如果按下“jump”则切换到 JumpState
        if (input_manager.isActionJustPressed("jump")) {
            return std::make_unique<JumpState>(player_component_);
        }

        // 步行状态可以左右移动
        if (input_manager.isActionHeldDown("move_left")) {
            physics_component->velocity_.x = std::min(physics_component->velocity_.x, 0.0F);
            // 添加向左的水平力
            physics_component->addForce({-player_component_->getMoveForce(), 0.0F});
            sprite_component->setFlipped(true); // 向左移动时翻转
        } else if (input_manager.isActionHeldDown("move_right")) {
            physics_component->velocity_.x = std::max(physics_component->velocity_.x, 0.0F);
            // 添加向右的水平力
            physics_component->addForce({player_component_->getMoveForce(), 0.0F});
            sprite_component->setFlipped(false); // 向右移动时不翻转
        } else {
            // 如果没有按下左右移动键，则切换到 IdleState
            return std::make_unique<IdleState>(player_component_);
        }
        return nullptr;
    }

    std::unique_ptr<PlayerState> WalkState::update(float /*unused*/, engine::core::Context & /*unused*/)
    {
        // 限制最大速度
        auto* physics_component = player_component_->getPhysicsComponent();
        auto max_speed = player_component_->getMaxSpeed();
        physics_component->velocity_.x =
            glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

        // 如果下方没有碰撞，则切换到 FallState
        if (!player_component_->isOnGround()) {
            return std::make_unique<FallState>(player_component_);
        }

        return nullptr;
    }

} // namespace game::component::player_state
