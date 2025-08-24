#include "parallax_component.h"
#include "../core/context.h"
#include "../object/game_object.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../render/sprite.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::component {
    ParallaxComponent::ParallaxComponent(std::string_view texture_id, glm::vec2 scroll_factor,
                                         glm::bvec2 repeat_xy)
        : sprite_(texture_id)
        , scroll_factor_(scroll_factor)
        , repeat_xy_(repeat_xy)
    {
        spdlog::trace("ParallaxComponent 构造完成，纹理 ID: {}", texture_id);
    }

    void ParallaxComponent::init()
    {
        if (owner_ == nullptr) {
            spdlog::error("ParallaxComponent 在初始化前未设置 owner_。");
            return;
        }
        transform_component_ = owner_->getComponent<TransformComponent>();
        if (transform_component_ == nullptr) {
            spdlog::warn(
                "GameObject '{}' 上的 ParallaxComponent 需要一个 TransformComponent，但未找到。",
                owner_->getName());
            return;
        }
    }

    void ParallaxComponent::render(engine::core::Context &context)
    {
        if (!is_visible_ || (transform_component_ == nullptr)) {
            return;
        }
        // 直接调用视差滚动绘制函数
        context.getRenderer().drawParallax(context.getCamera(), sprite_,
                                           transform_component_->getPosition(), scroll_factor_,
                                           repeat_xy_, transform_component_->getScale());
    }

} // namespace engine::component
