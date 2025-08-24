#pragma once
#include "../render/text_renderer.h"
#include "../utils/math.h"
#include "ui_element.h"
#include <string>
#include <string_view>

namespace engine::ui {

    /**
     * @brief UILabel 类用于创建和管理用户界面中的文本标签
     *
     * UILabel 继承自 UIElement，提供了文本渲染功能。
     * 它可以设置文本内容、字体ID、字体大小和文本颜色。
     *
     * @note 需要一个文本渲染器来获取和更新文本尺寸。
     */
    class UILabel final : public UIElement {
    private:
        engine::render::TextRenderer
            &text_renderer_;  ///< @brief 需要文本渲染器，用于获取/更新文本尺寸

        std::string text_;    ///< @brief 文本内容
        std::string font_id_; ///< @brief 字体ID
        int font_size_;       ///< @brief 字体大小
        engine::utils::FColor text_fcolor_ = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F};
        /* 可添加其他内容，例如边框、底色 */

    public:
        /**
         * @brief 构造一个UILabel
         *
         * @param text_renderer 文本渲染器
         * @param text 文本内容
         * @param font_id 字体ID
         * @param font_size 字体大小
         * @param text_color 文本颜色
         */
        UILabel(engine::render::TextRenderer &text_renderer, std::string_view text,
                std::string_view font_id, int font_size = 16,
                engine::utils::FColor text_color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
                glm::vec2 position = {0.0F, 0.0F});

        // --- 核心逻辑 ---
        void render(engine::core::Context &context) override;

        // --- getters ---
        [[nodiscard]] std::string_view getText() const { return text_; }
        [[nodiscard]] std::string_view getFontId() const { return font_id_; }
        [[nodiscard]] int getFontSize() const { return font_size_; }
        [[nodiscard]] const engine::utils::FColor &getTextFColor() const { return text_fcolor_; }

        // --- setters ---
        void setText(std::string_view text);
        void setFontId(std::string_view font_id);
        void setFontSize(int font_size);
        void setTextFColor(engine::utils::FColor text_fcolor);
    };

} // namespace engine::ui