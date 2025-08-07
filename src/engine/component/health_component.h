#pragma once
#include "../../engine/component/component.h"

namespace engine::component {
    /**
     * @brief 管理 GameObject 的生命值，处理伤害、治疗，并提供无敌帧功能。
     */
    class HealthComponent final : public Component {
        friend class engine::object::GameObject;

    private:
        int max_health_ = 1;                  ///< @brief 最大生命值
        int current_health_ = 1;              ///< @brief 当前生命值
        bool is_invincible_ = false;          ///< @brief 是否处于无敌状态
        float invincibility_duration_ = 2.0f; ///< @brief 受伤后无敌状态持续时间（秒）
        float invincibility_timer_ = 0.0f;    ///< @brief 当前无敌状态计时器（秒）

    public:
        /**
         * @brief 构造函数
         * @param max_health 最大生命值，默认为 1
         * @param invincibility_duration 无敌状态持续时间，默认为 2.0 秒
         */
        explicit HealthComponent(int max_health = 1, float invincibility_duration = 2.0f);
        ~HealthComponent() override = default;

        // 禁止拷贝和移动语义
        HealthComponent(const HealthComponent &) = delete;
        HealthComponent &operator=(const HealthComponent &) = delete;
        HealthComponent(HealthComponent &&) = delete;
        HealthComponent &operator=(HealthComponent &&) = delete;

        bool takeDamage(int damage_amount);
        void heal(int heal_amount);

        // --- getters ---
        int getMaxHealth() const { return max_health_; }
        int getCurrentHealth() const { return current_health_; }
        bool isInvincible() const { return is_invincible_; }
        bool isAlive() const { return current_health_ > 0; }

        // --- setters ---
        void setMaxHealth(int max_health);
        void setCurrentHealth(int current_health);
        void setInvincible(float duration);
        void setInvincibilityDuration(float duration) { invincibility_duration_ = duration; }

    protected:
        // 核心逻辑
        void update(float, engine::core::Context &) override;
    };

} // namespace engine::component
