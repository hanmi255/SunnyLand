#pragma once
#include "../render/sprite.h"
#include "ui_element.h"
#include <SDL3/SDL_rect.h>
#include <optional>
#include <string>
#include <string_view>

namespace engine::ui {

    /**
    * @brief 一个用于显示纹理或部分纹理的UI元素。
    *
    * 继承自UIElement并添加了渲染图像的功能。
    */
    class UIImage final : public UIElement {
    protected:
        engine::render::Sprite sprite_;

    public:
        /**
        * @brief 构造一个UIImage对象。
        *
        * @param texture_id 要显示的纹理ID。
        * @param position 图像的局部位置。
        * @param size 图像元素的大小。（如果为{0,0}，则使用纹理的原始尺寸）
        * @param source_rect
        * 可选：要绘制的纹理部分。（如果为空，则使用纹理的整个区域）
        * @param is_flipped 可选：精灵是否应该水平翻转。
        */
        UIImage(const std::string_view &texture_id,
                const glm::vec2 &position = {0.0f, 0.0f},
                const glm::vec2 &size = {0.0f, 0.0f},
                const std::optional<SDL_FRect> &source_rect = std::nullopt,
                bool is_flipped = false);

        // --- 核心方法 ---
        void render(engine::core::Context &context) override;

        // --- getters ---
        const engine::render::Sprite &getSprite() const { return sprite_; }
        const std::string_view getTextureId() const { return sprite_.getTextureId(); }
        const std::optional<SDL_FRect> &getSrcRect() const {
            return sprite_.getSrcRect();
        }

        // --- setters ---
        void setSprite(const engine::render::Sprite &sprite) { sprite_ = sprite; }
        void setTextureId(const std::string &texture_id) {
            sprite_.setTextureId(texture_id);
        }
        void setSrcRect(const std::optional<SDL_FRect> &source_rect) {
            sprite_.setSrcRect(source_rect);
        }

        bool isFlipped() const { return sprite_.isFlipped(); }
        void setFlipped(bool flipped) { sprite_.setFlipped(flipped); }
    };

} // namespace engine::ui