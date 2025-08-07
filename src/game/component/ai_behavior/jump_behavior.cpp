#include "jump_behavior.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/object/game_object.h"
#include "../ai_component.h"
#include <spdlog/spdlog.h>

namespace game::component::ai_behavior {

    JumpBehavior::JumpBehavior(float min_x, float max_x, glm::vec2 jump_vel, float jump_interval)
        : patrol_min_x_(min_x)
        , patrol_max_x_(max_x)
        , jump_vel_(jump_vel)
        , jump_interval_(jump_interval)
    {
        if (patrol_min_x_ >= patrol_max_x_) { // 确保巡逻范围是有效的
            spdlog::error("JumpBehavior: min_x ({}) 应小于 max_x ({})。行为可能不正确。", min_x,
                          max_x);
            patrol_min_x_ = patrol_max_x_;
        }
        if (jump_interval_ <= 0.0f) { // 确保跳跃间隔是正数
            spdlog::error("JumpBehavior: jump_interval ({}) 应为正数。已设置为 2.0f。",
                          jump_interval);
            jump_interval_ = 2.0f;
        }
        if (jump_vel_.y > 0) { // 确保垂直跳跃速度是负数（向上）
            spdlog::error("JumpBehavior: 垂直跳跃速度 ({}) 应为负数（向上）。已取相反数。",
                          jump_vel_.y);
            jump_vel_.y = -jump_vel_.y;
        }
    }

    void JumpBehavior::update(float delta_time, AIComponent &ai_component)
    {
        // 获取必要的组件
        auto* physics_component = ai_component.getPhysicsComponent();
        auto* transform_component = ai_component.getTransformComponent();
        auto* sprite_component = ai_component.getSpriteComponent();
        auto* animation_component = ai_component.getAnimationComponent();

        if (!physics_component || !transform_component || !sprite_component ||
            !animation_component) {
            spdlog::error("JumpBehavior：缺少必要的组件，无法执行跳跃行为。");
            return;
        }

        const auto is_on_ground = physics_component->hasCollidedBelow();

        // 不在地面上，根据垂直速度判断是上升还是下落
        if (!is_on_ground) {
            if (physics_component->getVelocity().y < 0) {
                animation_component->playAnimation("jump");
            } else {
                animation_component->playAnimation("fall");
            }
            return;
        }

        // 在地面上的处理逻辑
        jump_timer_ += delta_time;
        physics_component->velocity_.x = 0.0f;

        // 检查是否需要跳跃
        if (jump_timer_ < jump_interval_) {
            // 还在地面等待
            animation_component->playAnimation("idle");
            return;
        }

        // 准备跳跃
        jump_timer_ = 0.0f;

        // 检查是否需要更新跳跃方向
        const auto current_x = transform_component->getPosition().x;

        // 如果右边超限或者撞墙，向左跳
        if (jumping_right_ &&
            (physics_component->hasCollidedRight() || current_x >= patrol_max_x_)) {
            jumping_right_ = false;
        }
        // 如果左边超限或者撞墙，向右跳
        else if (!jumping_right_ &&
                 (physics_component->hasCollidedLeft() || current_x <= patrol_min_x_)) {
            jumping_right_ = true;
        }

        const auto jump_vel_x = jumping_right_ ? jump_vel_.x : -jump_vel_.x;
        physics_component->velocity_ = {jump_vel_x, jump_vel_.y};
        animation_component->playAnimation("jump");
        sprite_component->setFlipped(jumping_right_);
    }

} // namespace game::component::ai_behavior
