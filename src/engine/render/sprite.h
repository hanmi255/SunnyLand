/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 18:18:46
 * @LastEditTime: 2025-07-29 18:31:49
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\render\sprite.h
 * @技术宅拯救世界！！！
 */
#pragma once
#include <SDL3/SDL_rect.h> // 用于 SDL_FRect
#include <optional>        // 用于 std::optional
#include <string>          // 用于 std::string
#include <string_view>

namespace engine::render {
    /**
     * @brief 表示要绘制的精灵数据
     *
     * 包含纹理标识符、要绘制的纹理部分（源矩形）和翻转状态
     * 位置、缩放和旋转由外部模块控制（例如 SpriteComponent）
     * 渲染由 Render 类完成，传入 Sprite 作为参数
     */
    class Sprite final {
    private:
        std::string texture_id_;            ///< @brief 纹理标识符
        std::optional<SDL_FRect> src_rect_; ///< @brief (可选)源矩形
        bool is_flipped_ = false;           ///< @brief 是否翻转

    public:
        /**
         * @brief 默认构造函数（创建一个空的/无效的精灵）
         */
        Sprite() = default;

        /**
         * @brief 构造一个精灵
         *
         * @param texture_id 纹理资源的标识符。不应为空。
         * @param source_rect 可选的源矩形（SDL_FRect），定义要使用的纹理部分。如果为
         * std::nullopt，则使用整个纹理。
         * @param is_flipped 是否水平翻转
         */
        explicit Sprite(std::string_view texture_id, const std::optional<SDL_FRect> &src_rect = std::nullopt,
               bool is_flipped = false)
            : texture_id_(texture_id), src_rect_(src_rect), is_flipped_(is_flipped)
        {}

        // --- getters ---
        [[nodiscard]] std::string_view getTextureId() const { return texture_id_; }
        [[nodiscard]] const std::optional<SDL_FRect> &getSrcRect() const { return src_rect_; }
        [[nodiscard]] bool isFlipped() const { return is_flipped_; }

        // --- setters ---
        void setTextureId(std::string_view texture_id) { texture_id_ = texture_id; }
        void setSrcRect(std::optional<SDL_FRect> src_rect) { src_rect_ = src_rect; }
        void setFlipped(bool flipped) { is_flipped_ = flipped; }
    };

} // namespace engine::render
