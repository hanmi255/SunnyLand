/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 18:39:26
 * @LastEditTime: 2025-07-30 17:49:15
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\render\camera.cpp
 * @技术宅拯救世界！！！
 */
#include "camera.h"
#include "../component/transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::render {
    Camera::Camera(glm::vec2 viewport_size, glm::vec2 position,
                   std::optional<engine::utils::Rect> limit_bounds)
        : viewport_size_(std::move(viewport_size))
        , viewport_center_offset_(viewport_size_ * 0.5f)
        , position_(std::move(position))
        , limit_bounds_(std::move(limit_bounds))
    {
        spdlog::trace("Camera 初始化成功，位置: {},{}", position_.x, position_.y);
    }

    void Camera::update(float delta_time)
    {
        if (target_ == nullptr) return;

        glm::vec2 desired_position = target_->getPosition() - viewport_center_offset_;
        glm::vec2 offset = desired_position - position_;

        // 快速距离检查：使用曼哈顿距离或切比雪夫距离
        if (std::abs(offset.x) < 1.0f && std::abs(offset.y) < 1.0f) {
            position_ = desired_position;
        } else {
            position_ += offset * (smooth_factor_ * delta_time);
            position_ = glm::round(position_);
        }

        clampPosition();
    }

    void Camera::move(const glm::vec2 &offset)
    {
        position_ += offset;
        clampPosition();
    }

    glm::vec2 Camera::worldToScreen(const glm::vec2 &world_pos) const
    {
        return world_pos - position_;
    }

    glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2 &world_pos,
                                                const glm::vec2 &scroll_factor) const
    {
        return world_pos - position_ * scroll_factor;
    }

    glm::vec2 Camera::screenToWorld(const glm::vec2 &screen_pos) const
    {
        return screen_pos + position_;
    }

    const glm::vec2 Camera::getPosition() const
    {
        return position_;
    }

    std::optional<engine::utils::Rect> Camera::getLimitBounds() const
    {
        return limit_bounds_;
    }

    glm::vec2 Camera::getViewportSize() const
    {
        return viewport_size_;
    }

    engine::component::TransformComponent* Camera::getTarget() const
    {
        return target_;
    }

    void Camera::setPostion(const glm::vec2 &position)
    {
        position_ = position;
        clampPosition();
    }

    void Camera::setLimitBounds(const engine::utils::Rect &bounds)
    {
        limit_bounds_ = bounds;
        clampPosition();
    }

    void Camera::setTarget(engine::component::TransformComponent* target)
    {
        target_ = target;
    }

    void Camera::clampPosition()
    {
        if (!limit_bounds_.has_value() || limit_bounds_->size.x <= 0 ||
            limit_bounds_->size.y <= 0) {
            return;
        }

        glm::vec2 min_pos = limit_bounds_->position;
        glm::vec2 max_pos =
            glm::max(min_pos, limit_bounds_->position + limit_bounds_->size - viewport_size_);
        position_ = glm::clamp(position_, min_pos, max_pos);
    }
} // namespace engine::render
