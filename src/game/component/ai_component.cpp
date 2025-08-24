#include "ai_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/audio_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/object/game_object.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component {

    void AIComponent::init()
    {
        if (owner_ == nullptr) {
            spdlog::error("AIComponent 在初始化前未设置 owner_。");
            return;
        }

        // 获取并缓存必要的组件指针
        transform_component_ = owner_->getComponent<engine::component::TransformComponent>();
        physics_component_ = owner_->getComponent<engine::component::PhysicsComponent>();
        sprite_component_ = owner_->getComponent<engine::component::SpriteComponent>();
        animation_component_ = owner_->getComponent<engine::component::AnimationComponent>();
        audio_component_ = owner_->getComponent<engine::component::AudioComponent>();

        // 检查是否所有必需的组件都存在（音频组件非必需）
        if ((transform_component_ == nullptr) || (physics_component_ == nullptr) ||
            (sprite_component_ == nullptr) || (animation_component_ == nullptr)) {
            spdlog::error("GameObject '{}' 上的 AIComponent 缺少必需的组件", owner_->getName());
        }
    }

    void AIComponent::update(float delta_time, engine::core::Context & /*unused*/)
    {
        // 将更新委托给当前的行为策略
        if (current_behavior_) {
            current_behavior_->update(delta_time, *this);
        } else {
            spdlog::warn("GameObject '{}' 上的 AIComponent 没有设置行为。",
                         (owner_ != nullptr) ? owner_->getName() : "Unknown");
        }
    }

    void AIComponent::setBehavior(std::unique_ptr<ai_behavior::AIBehavior> behavior)
    {
        current_behavior_ = std::move(behavior);
        spdlog::debug("GameObject '{}' 上的 AIComponent 设置了新的行为。",
                      (owner_ != nullptr) ? owner_->getName() : "Unknown");
        if (current_behavior_) {
            current_behavior_->enter(*this);
        }
    }

    bool AIComponent::takeDamage(int damage_amount)
    {
        bool success = false;
        if (auto* health_component = getOwner()->getComponent<engine::component::HealthComponent>();
            health_component) {
            success = health_component->takeDamage(damage_amount);
            // TODO: 可以设置受伤/死亡后的行为
        }
        return success;
    }

    bool AIComponent::isAlive() const
    {
        if (auto* health_component = getOwner()->getComponent<engine::component::HealthComponent>();
            health_component) {
            return health_component->isAlive();
        }
        return true;
    }

} // namespace game::component
