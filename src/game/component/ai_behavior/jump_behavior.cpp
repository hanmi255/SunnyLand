#include "jump_behavior.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
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
        if (jump_interval_ <= 0.0F) { // 确保跳跃间隔是正数
            spdlog::error("JumpBehavior: jump_interval ({}) 应为正数。已设置为 2.0f。",
                          jump_interval);
            jump_interval_ = 2.0F;
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
        auto* audio_component = ai_component.getAudioComponent();

        // 检查必要组件
        if ((physics_component == nullptr) || (transform_component == nullptr) ||
            (sprite_component == nullptr) || (animation_component == nullptr)) {
            spdlog::error("JumpBehavior：缺少必要的组件，无法执行跳跃行为。");
            return;
        }

        const auto is_on_ground = physics_component->hasCollidedBelow();

        // 不在地面上时的处理
        if (!is_on_ground) {
            if (physics_component->getVelocity().y < 0) {
                animation_component->playAnimation("jump");
            } else {
                animation_component->playAnimation("fall");
            }
            return;
        }

        // 播放落地音效
        if ((audio_component != nullptr) && jump_timer_ < 0.001F) {
            audio_component->playSound("cry", -1, true);
        }

        // 更新计时器和速度
        jump_timer_ += delta_time;
        physics_component->velocity_.x = 0.0F;

        // 还在地面等待时
        if (jump_timer_ < jump_interval_) {
            animation_component->playAnimation("idle");
            return;
        }

        // 准备跳跃
        jump_timer_ = 0.0F;

        // 更新跳跃方向
        const auto current_x = transform_component->getPosition().x;

        if (jumping_right_ &&
            (physics_component->hasCollidedRight() || current_x >= patrol_max_x_)) {
            jumping_right_ = false;
        } else if (!jumping_right_ &&
                   (physics_component->hasCollidedLeft() || current_x <= patrol_min_x_)) {
            jumping_right_ = true;
        }

        // 执行跳跃
        const auto jump_vel_x = jumping_right_ ? jump_vel_.x : -jump_vel_.x;
        physics_component->velocity_ = {jump_vel_x, jump_vel_.y};
        animation_component->playAnimation("jump");
        sprite_component->setFlipped(jumping_right_);
    }

} // namespace game::component::ai_behavior
