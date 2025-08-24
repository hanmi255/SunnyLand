#include "patrol_behavior.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
#include "../ai_component.h"
#include <spdlog/spdlog.h>

namespace game::component::ai_behavior {

    PatrolBehavior::PatrolBehavior(float min_x, float max_x, float speed)
        : patrol_min_x_(min_x), patrol_max_x_(max_x), move_speed_(speed)
    {
        if (patrol_min_x_ >= patrol_max_x_) {
            spdlog::error("PatrolBehavior：min_x ({}) 应小于 max_x ({})。行为可能不正确。", min_x,
                          max_x);
            patrol_min_x_ = patrol_max_x_;
        }
    }

    void PatrolBehavior::enter(AIComponent &ai_component)
    {
        if (auto* animation_component = ai_component.getAnimationComponent(); animation_component) {
            animation_component->playAnimation("walk");
        }
    }

    void PatrolBehavior::update(float /*delta_time*/, AIComponent &ai_component)
    {
        // 获取必要的组件
        auto* physics_component = ai_component.getPhysicsComponent();
        auto* transform_component = ai_component.getTransformComponent();
        auto* sprite_component = ai_component.getSpriteComponent();

        // 使用卫语句检查组件是否存在
        if ((physics_component == nullptr) || (transform_component == nullptr) || (sprite_component == nullptr)) {
            spdlog::error("PatrolBehavior：缺少必要的组件，无法执行巡逻行为。");
            return;
        }

        // 获取当前位置
        const auto current_x = transform_component->getPosition().x;

        // 撞右墙或到达设定目标则转向左
        if (physics_component->hasCollidedRight() || current_x >= patrol_max_x_) {
            physics_component->velocity_.x = -move_speed_;
            moving_right_ = false;
            sprite_component->setFlipped(moving_right_);
            return;
        }

        // 撞左墙或到达设定目标则转向右
        if (physics_component->hasCollidedLeft() || current_x <= patrol_min_x_) {
            physics_component->velocity_.x = move_speed_;
            moving_right_ = true;
            sprite_component->setFlipped(moving_right_);
            return;
        }

        // 更新精灵翻转(向左移动时，不翻转)
        sprite_component->setFlipped(moving_right_);
    }

} // namespace game::component::ai_behavior
