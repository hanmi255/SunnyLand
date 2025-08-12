#include "animation.h"
#include <algorithm>
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::render {
    Animation::Animation(std::string_view name, bool loop) : name_(name), loop_(loop) {}

    void Animation::addFrame(SDL_FRect src_rect, float duration)
    {
        if (duration <= 0.0f) {
            spdlog::warn("尝试向动画 '{}' 添加无效持续时间的帧", name_);
            return;
        }
        frames_.push_back({src_rect, duration});
        total_duration_ += duration;
    }

    const AnimationFrame &Animation::getFrame(float time) const
    {
        // 处理空帧情况
        if (frames_.empty()) {
            static const AnimationFrame empty_frame = {{0, 0, 0, 0}, 0.0f};
            spdlog::error("动画 '{}' 没有帧，无法获取帧", name_);
            return empty_frame;
        }

        // 对于只有一个帧的情况进行优化
        if (frames_.size() == 1) {
            return frames_.front();
        }

        float current_time = time;

        if (loop_ && total_duration_ > 0.0f) {
            // 对循环动画使用模运算获取有效时间
            current_time = glm::mod(time, total_duration_);
        } else {
            // 对于非循环动画，如果时间超过总时长，则停留在最后一帧
            if (current_time >= total_duration_) {
                return frames_.back();
            }
        }

        // 使用二分查找找到正确的帧，提高性能
        float accumulated_time = 0.0f;
        auto it = std::find_if(frames_.begin(), frames_.end(),
                               [&accumulated_time, current_time](const AnimationFrame &frame) {
                                   accumulated_time += frame.duration;
                                   return current_time < accumulated_time;
                               });

        // 理论上不应到达这里，但如果查找失败，返回最后一帧
        if (it == frames_.end()) {
            spdlog::warn("动画 '{}' 在获取帧信息时出现错误。", name_);
            return frames_.back();
        }

        return *it;
    }
} // namespace engine::render
