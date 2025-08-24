#pragma once
#include "../../engine/component/component.h"
#include "ai_behavior/ai_behavior.h"
#include <memory>

namespace game::component::ai_behavior {
    class AIBehavior;
} // namespace game::component::ai_behavior

namespace engine::component {
    class AnimationComponent;
    class AudioComponent;
    class PhysicsComponent;
    class SpriteComponent;
    class TransformComponent;
} // namespace engine::component

namespace game::component {

    /**
     * @brief 负责管理 GameObject 的 AI 行为。
     *
     * 使用策略模式，持有一个具体的 AIBehavior 实例来执行实际的 AI 逻辑。
     * 提供对 GameObject 其他关键组件的访问。
     */
    class AIComponent final : public engine::component::Component {
        friend class engine::object::GameObject;

    private:
        std::unique_ptr<ai_behavior::AIBehavior> current_behavior_ =
            nullptr; ///< @brief 当前 AI 行为

        engine::component::AnimationComponent* animation_component_ = nullptr; ///< @brief 动画组件
        engine::component::AudioComponent* audio_component_ = nullptr;         ///< @brief 音频组件
        engine::component::PhysicsComponent* physics_component_ = nullptr;     ///< @brief 物理组件
        engine::component::SpriteComponent* sprite_component_ = nullptr;       ///< @brief 精灵组件
        engine::component::TransformComponent* transform_component_ = nullptr; ///< @brief 变换组件

    public:
        AIComponent() = default;
        ~AIComponent() override = default;

        // 禁止拷贝和移动
        AIComponent(const AIComponent &) = delete;
        AIComponent &operator=(const AIComponent &) = delete;
        AIComponent(AIComponent &&) = delete;
        AIComponent &operator=(AIComponent &&) = delete;

        void setBehavior(std::unique_ptr<ai_behavior::AIBehavior> behavior);
        bool takeDamage(int damage_amount);
        [[nodiscard]] bool isAlive() const;

        // --- getters ---
        [[nodiscard]] engine::component::AnimationComponent* getAnimationComponent() const
        {
            return animation_component_;
        }
        [[nodiscard]] engine::component::AudioComponent* getAudioComponent() const { return audio_component_; }
        [[nodiscard]] engine::component::PhysicsComponent* getPhysicsComponent() const
        {
            return physics_component_;
        }
        [[nodiscard]] engine::component::SpriteComponent* getSpriteComponent() const { return sprite_component_; }
        [[nodiscard]] engine::component::TransformComponent* getTransformComponent() const
        {
            return transform_component_;
        }

    private:
        // 核心逻辑
        void init() override;
        void update(float delta_time, engine::core::Context & /*unused*/) override;
    };
} // namespace game::component
