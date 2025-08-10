#include "collider_component.h"
#include "../object/game_object.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::component {

    ColliderComponent::ColliderComponent(std::unique_ptr<engine::physics::Collider> collider,
                                         engine::utils::Alignment alignment, bool is_trigger,
                                         bool is_active)
        : collider_(std::move(collider))
        , alignment_(alignment)
        , is_trigger_(is_trigger)
        , is_active_(is_active)
    {
        if (!collider_) {
            spdlog::error("创建 ColliderComponent 时传入了空的碰撞器！");
        }
    }

    void ColliderComponent::init()
    {
        if (!owner_) {
            spdlog::error("ColliderComponent 在初始化前未设置 owner_。");
            return;
        }
        transform_component_ = owner_->getComponent<TransformComponent>();
        if (!transform_component_) {
            spdlog::warn(
                "GameObject '{}' 上的 ColliderComponent 需要一个 TransformComponent，但未找到。",
                owner_->getName());
            return;
        }

        updateOffset();
    }

    void ColliderComponent::setAlignment(engine::utils::Alignment anchor)
    {
        alignment_ = anchor;
        // 重新计算偏移量，确保 transform_component_ 和 collider_ 有效
        if (transform_component_ && collider_) {
            updateOffset();
        }
    }

    void ColliderComponent::updateOffset()
    {
        if (!collider_) return;

        // 获取碰撞盒的最小包围盒尺寸
        auto collider_size = collider_->getAABBSize();

        // 如果尺寸无效，偏移为0
        if (collider_size.x <= 0.0f || collider_size.y <= 0.0f) {
            offset_ = {0.0f, 0.0f};
            return;
        }
        auto scale = transform_component_->getScale();

        // 根据 alignment_anchor_ 计算 AABB 左上角相对于 Transform 中心的偏移量
        switch (alignment_) {
            case engine::utils::Alignment::TOP_LEFT:
                offset_ = glm::vec2{0.0f, 0.0f} * scale;
                break;
            case engine::utils::Alignment::TOP_CENTER:
                offset_ = glm::vec2{-collider_size.x / 2.0f, 0.0f} * scale;
                break;
            case engine::utils::Alignment::TOP_RIGHT:
                offset_ = glm::vec2{-collider_size.x, 0.0f} * scale;
                break;
            case engine::utils::Alignment::CENTER_LEFT:
                offset_ = glm::vec2{0.0f, -collider_size.y / 2.0f} * scale;
                break;
            case engine::utils::Alignment::CENTER:
                offset_ = glm::vec2{-collider_size.x / 2.0f, -collider_size.y / 2.0f} * scale;
                break;
            case engine::utils::Alignment::CENTER_RIGHT:
                offset_ = glm::vec2{-collider_size.x, -collider_size.y / 2.0f} * scale;
                break;
            case engine::utils::Alignment::BOTTOM_LEFT:
                offset_ = glm::vec2{0.0f, -collider_size.y} * scale;
                break;
            case engine::utils::Alignment::BOTTOM_CENTER:
                offset_ = glm::vec2{-collider_size.x / 2.0f, -collider_size.y} * scale;
                break;
            case engine::utils::Alignment::BOTTOM_RIGHT:
                offset_ = glm::vec2{-collider_size.x, -collider_size.y} * scale;
                break;
            default:
                break; // 如果 alignment_ 是 NONE，则不做任何操作（手动设置 offset_）
        }
    }

    engine::utils::Rect ColliderComponent::getWorldAABB() const
    {
        if (!transform_component_ || !collider_) {
            return {glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f)};
        }
        // 计算最小包围盒的左上角坐标（position）
        const glm::vec2 top_left_pos = transform_component_->getPosition() + offset_;
        // 计算最小包围盒的尺寸（size）
        const glm::vec2 base_size = collider_->getAABBSize();
        const glm::vec2 scale = transform_component_->getScale();
        glm::vec2 scaled_size = base_size * scale;
        // 返回最小包围盒的 Rect
        return {top_left_pos, scaled_size};
    }
} // namespace engine::component
