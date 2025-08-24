#include "health_component.h"
#include "../../engine/object/game_object.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::component {

    HealthComponent::HealthComponent(int max_health, float invincibility_duration)
        : max_health_(glm::max(1, max_health))
        , // 确保最大生命值至少为 1
        current_health_(max_health_)
        , // 初始化当前生命值为最大生命值
        invincibility_duration_(invincibility_duration)
    {
    }

    void HealthComponent::update(float delta_time, engine::core::Context & /*unused*/)
    {
        if (!is_invincible_) {
            return;
        }

        invincibility_timer_ -= delta_time;
        if (invincibility_timer_ <= 0.0F) {
            is_invincible_ = false;
            invincibility_timer_ = 0.0F;
        }
    }

    bool HealthComponent::takeDamage(int damage_amount)
    {
        if (damage_amount <= 0 || !isAlive()) {
            return false;
        }

        if (is_invincible_) {
            spdlog::debug("游戏对象 '{}' 处于无敌状态，免疫了 {} 点伤害。",
                          (owner_ != nullptr) ? owner_->getName() : "Unknown", damage_amount);
            return false;
        }

        current_health_ -= damage_amount;
        current_health_ = glm::max(0, current_health_); // 防止生命值变为负数
        // 如果受伤但没死亡，并且设置了无敌时间，则触发无敌
        if (isAlive() && invincibility_duration_ > 0.0F) {
            setInvincible(invincibility_duration_);
        }
        spdlog::debug("游戏对象 '{}' 受到了 {} 点伤害，当前生命值: {}/{}。",
                      (owner_ != nullptr) ? owner_->getName() : "Unknown", damage_amount, current_health_,
                      max_health_);
        return true;
    }

    int HealthComponent::heal(int heal_amount)
    {
        if (heal_amount <= 0 || !isAlive()) {
            return current_health_;
        }

        current_health_ += heal_amount;
        current_health_ = std::min(max_health_, current_health_); // 防止超过最大生命值
        spdlog::debug("游戏对象 '{}' 治疗了 {} 点，当前生命值: {}/{}。",
                      (owner_ != nullptr) ? owner_->getName() : "Unknown", heal_amount, current_health_,
                      max_health_);
        return current_health_;
    }

    void HealthComponent::setMaxHealth(int max_health)
    {
        max_health_ = glm::max(1, max_health);                    // 确保最大生命值至少为 1
        current_health_ = glm::min(current_health_, max_health_); // 确保当前生命值不超过最大生命值
    }

    void HealthComponent::setCurrentHealth(int current_health)
    {
        // 确保当前生命值在 0 到最大生命值之间
        current_health_ = glm::max(0, glm::min(current_health, max_health_));
    }

    void HealthComponent::setInvincible(float duration)
    {
        if (duration > 0.0F) {
            is_invincible_ = true;
            invincibility_timer_ = duration;
            spdlog::debug("游戏对象 '{}' 进入无敌状态，持续 {} 秒。",
                          (owner_ != nullptr) ? owner_->getName() : "Unknown", duration);
        } else {
            // 如果持续时间为 0 或负数，则立即取消无敌
            is_invincible_ = false;
            invincibility_timer_ = 0.0F;
            spdlog::debug("游戏对象 '{}' 的无敌状态被手动移除。",
                          (owner_ != nullptr) ? owner_->getName() : "Unknown");
        }
    }
} // namespace engine::component
