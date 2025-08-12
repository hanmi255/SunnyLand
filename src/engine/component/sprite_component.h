#pragma once
#include "../render/sprite.h"
#include "../utils/alignment.h"
#include "component.h"
#include <SDL3/SDL_rect.h>
#include <glm/vec2.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::resource {
    class ResourceManager;
} // namespace engine::resource

namespace engine::component {
    class TransformComponent;

    class SpriteComponent final : public engine::component::Component {
        friend class engine::object::GameObject;

    private:
        engine::resource::ResourceManager* resource_manager_ =
            nullptr;                           ///< @brief 资源管理器指针，用于获取纹理大小
        TransformComponent* transform_component_ =
            nullptr;                           ///< @brief 变换组件指针，用于获取位置和缩放(非必须)

        engine::render::Sprite sprite_;        ///< @brief 精灵
        engine::utils::Alignment alignment_ =
            engine::utils::Alignment::NONE;    ///< @brief 精灵对齐方式
        glm::vec2 sprite_size_ = {0.0f, 0.0f}; ///< @brief 精灵大小
        glm::vec2 offset_ = {0.0f, 0.0f};      ///< @brief 偏移量
        bool is_visible_ = true;               ///< @brief 是否可见

    public:
        /**
         * @brief 构造函数
         * @param texture_id 纹理资源的标识符。
         * @param resource_manager 资源管理器指针。
         * @param alignment 初始对齐方式。
         * @param source_rect_opt 可选的源矩形。
         * @param is_flipped 初始翻转状态。
         */
        SpriteComponent(const std::string &texture_id,
                        engine::resource::ResourceManager &resource_manager,
                        engine::utils::Alignment alignment = engine::utils::Alignment::NONE,
                        const std::optional<SDL_FRect> source_rect_opt = std::nullopt,
                        bool is_flipped = false);

        /**
         * @brief 构造函数
         * @param sprite 精灵对象。
         * @param resource_manager 资源管理器指针。
         * @param alignment 初始对齐方式。
         */
        SpriteComponent(engine::render::Sprite &&sprite,
                        engine::resource::ResourceManager &resource_manager,
                        engine::utils::Alignment alignment = engine::utils::Alignment::NONE);

        ~SpriteComponent() override = default;

        // 禁止拷贝和移动语义
        SpriteComponent(const SpriteComponent &) = delete;
        SpriteComponent &operator=(const SpriteComponent &) = delete;
        SpriteComponent(SpriteComponent &&) = delete;
        SpriteComponent &operator=(SpriteComponent &&) = delete;

        void updateOffset();

        // --- getters ---
        const engine::render::Sprite &getSprite() const { return sprite_; }
        std::string_view getTextureId() const { return sprite_.getTextureId(); }
        bool isFlipped() const { return sprite_.isFlipped(); }
        bool isVisible() const { return is_visible_; }
        const glm::vec2 &getSpriteSize() const { return sprite_size_; }
        const glm::vec2 &getOffset() const { return offset_; }
        engine::utils::Alignment getAlignment() const { return alignment_; }

        // --- setters ---
        void setSpriteById(const std::string &texture_id,
                           std::optional<SDL_FRect> source_rect_opt = std::nullopt);
        void setFlipped(bool flipped) { sprite_.setFlipped(flipped); }
        void setVisible(bool visible) { is_visible_ = visible; }
        void setSrcRect(std::optional<SDL_FRect> src_rect_pot);
        void setAlignment(engine::utils::Alignment anchor);

    private:
        void updateSpriteSize();

        void init() override;
        void update(float, engine::core::Context &) override {};
        void render(engine::core::Context &context) override;
    };
} // namespace engine::component
