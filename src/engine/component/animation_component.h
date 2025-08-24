#pragma once
#include "./component.h"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace engine::render {
    class Animation;
} // namespace engine::render

namespace engine::component {
    class SpriteComponent;
} // namespace engine::component

namespace engine::component {

    /**
     * @brief GameObject的动画组件。
     *
     * 持有一组Animation对象并控制其播放，
     * 根据当前帧更新关联的SpriteComponent。
     */
    class AnimationComponent : public Component {
        friend class engine::object::GameObject;

    private:
        std::unordered_map<std::string, std::unique_ptr<engine::render::Animation>>
            animations_;                              /// @brief 动画名称到Animation对象的映射。
        SpriteComponent* sprite_component_ = nullptr; ///< @brief 指向必需的SpriteComponent的指针
        engine::render::Animation* current_animation_ =
            nullptr;                                  ///< @brief 指向当前播放动画的原始指针

        float animation_timer_ = 0.0F;                ///< @brief 动画播放中的计时器
        bool is_playing_ = false;                     ///< @brief 当前是否有动画正在播放
        bool is_one_shot_removal_ = false;            ///< @brief 是否在动画结束后删除整个GameObject

    public:
        AnimationComponent() = default;
        ~AnimationComponent() override;

        // 禁止拷贝和移动语义
        AnimationComponent(const AnimationComponent &) = delete;
        AnimationComponent &operator=(const AnimationComponent &) = delete;
        AnimationComponent(AnimationComponent &&) = delete;
        AnimationComponent &operator=(AnimationComponent &&) = delete;

        void addAnimation(std::unique_ptr<engine::render::Animation> animation);
        void playAnimation(std::string_view name);
        void stopAnimation() { is_playing_ = false; }
        void resumeAnimation() { is_playing_ = true; }

        // --- getters ---
        [[nodiscard]] std::string_view getCurrentAnimationName() const;
        [[nodiscard]] bool isPlaying() const { return is_playing_; }
        [[nodiscard]] bool isAnimationFinished() const;
        [[nodiscard]] bool isOneShotRemoval() const { return is_one_shot_removal_; }

        // --- setters ---
        void setOneShotRemoval(bool is_one_shot_removal)
        {
            is_one_shot_removal_ = is_one_shot_removal;
        }

    protected:
        // 核心逻辑
        void init() override;
        void update(float /*unused*/, engine::core::Context & /*unused*/) override;
    };

} // namespace engine::component
