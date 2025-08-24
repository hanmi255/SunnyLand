#pragma once
#include <SDL3/SDL_rect.h>
#include <string>
#include <vector>

namespace engine::render {

    /**
     * @brief 代表动画中的单个帧。
     *
     * 包含纹理图集上的源矩形和该帧的显示持续时间。
     */
    struct AnimationFrame {
        SDL_FRect source_rect; ///< @brief 纹理图集上此帧的区域
        float duration;        ///< @brief 此帧显示的持续时间（秒）
    };

    /**
     * @brief 管理一系列动画帧。
     *
     * 存储动画的帧、总时长、名称和循环行为。
     */
    class Animation final {
    private:
        std::string name_;                   ///< @brief 动画名称
        std::vector<AnimationFrame> frames_; ///< @brief 动画帧
        float total_duration_ = 0.0F;        ///< @brief 动画总时长（秒）
        bool loop_ = true;                   ///< @brief 是否循环播放（默认循环）

    public:
        explicit Animation(std::string_view name = "default", bool loop = true);
        ~Animation() = default;

        // 禁止拷贝和移动语义
        Animation(const Animation &) = delete;
        Animation &operator=(const Animation &) = delete;
        Animation(Animation &&) = delete;
        Animation &operator=(Animation &&) = delete;

        /**
         * @brief 向动画添加一帧。
         *
         * @param src_rect 纹理图集上此帧的区域。
         * @param duration 此帧应显示的持续时间（秒）。
         */
        void addFrame(SDL_FRect src_rect, float duration);

        /**
         * @brief 获取在给定时间点应该显示的动画帧。
         * @param time 当前时间（秒）。如果动画循环，则可以超过总持续时间。
         * @return 对应时间点的动画帧。
         */
        [[nodiscard]] const AnimationFrame &getFrame(float time) const;

        // --- getters ---
        [[nodiscard]] std::string_view getName() const { return name_; }
        [[nodiscard]] const std::vector<AnimationFrame> &getFrames() const { return frames_; }
        [[nodiscard]] size_t getFrameCount() const { return frames_.size(); }
        [[nodiscard]] float getTotalDuration() const { return total_duration_; }
        [[nodiscard]] bool isLooping() const { return loop_; }
        [[nodiscard]] bool isEmpty() const { return frames_.empty(); }

        // --- setters ---
        void setName(std::string_view name) { name_ = name; }
        void setLooping(bool loop) { loop_ = loop; }
    };
} // namespace engine::render
