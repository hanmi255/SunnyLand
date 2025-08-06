#include "jump_state.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "fall_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component::state {

    void JumpState::enter()
    {
        playAnimation("jump");
        auto physics_component = player_component_->getPhysicsComponent();
        physics_component->velocity_.y = -player_component_->getJumpForce(); // 向上跳跃
        spdlog::debug("PlayerComponent 进入 JumpState，设置初始垂直速度为: {}",
                      physics_component->velocity_.y);
    }

    void JumpState::exit() {}

    std::unique_ptr<PlayerState> JumpState::handleInput(engine::core::Context &context)
    {
        auto input_manager = context.getInputManager();
        auto physics_component = player_component_->getPhysicsComponent();
        auto sprite_component = player_component_->getSpriteComponent();

        // 跳跃状态下可以左右移动
        if (input_manager.isActionHeldDown("move_left")) {
            if (physics_component->velocity_.x > 0.0f) physics_component->velocity_.x = 0.0f;
            physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
            sprite_component->setFlipped(true);
        } else if (input_manager.isActionHeldDown("move_right")) {
            if (physics_component->velocity_.x < 0.0f) physics_component->velocity_.x = 0.0f;
            physics_component->addForce({player_component_->getMoveForce(), 0.0f});
            sprite_component->setFlipped(false);
        }
        return nullptr;
    }

    std::unique_ptr<PlayerState> JumpState::update(float, engine::core::Context &)
    {
        // 限制最大速度(水平方向)
        auto physics_component = player_component_->getPhysicsComponent();
        auto max_speed = player_component_->getMaxSpeed();
        physics_component->velocity_.x =
            glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

        // 如果速度为正，切换到 FallState
        if (physics_component->velocity_.y > 0.0f) {
            return std::make_unique<FallState>(player_component_);
        }

        return nullptr;
    }

} // namespace game::component::state
