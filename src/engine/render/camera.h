/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 18:39:10
 * @LastEditTime: 2025-07-30 16:41:46
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\render\camera.h
 * @技术宅拯救世界！！！
 */
#pragma once
#include "../utils/math.h"
#include <optional>

namespace engine::render {

    /**
     * @brief 相机类负责管理相机位置和视口大小，并提供坐标转换功能。
     * 它还包含限制相机移动范围的边界。
     */
    class Camera final {
    private:
        glm::vec2 viewport_size_;          ///<@brief 视口大小
        glm::vec2 viewport_center_offset_; ///<@brief 视口中心偏移
        glm::vec2 position_;               ///<@brief 相机左上角的世界坐标
        std::optional<engine::utils::Rect>
            limit_bounds_;                 ///<@brief 限制相机移动范围，空值表示不限制

    public:
        /**
         * @brief 构造相机对象
         * @param viewport_size 视口大小
         * @param position 相机位置
         * @param limit_bounds 限制相机的移动范围
         */
        Camera(glm::vec2 viewport_size, glm::vec2 position = glm::vec2(0.0f, 0.0f),
               std::optional<engine::utils::Rect> limit_bounds = std::nullopt);

        // 禁用拷贝和移动语义
        Camera(const Camera &) = delete;
        Camera &operator=(const Camera &) = delete;
        Camera(Camera &&) = delete;
        Camera &operator=(Camera &&) = delete;

        void update(float delta_time);
        void move(const glm::vec2 &offset);

        glm::vec2 worldToScreen(const glm::vec2 &world_pos) const;
        glm::vec2 worldToScreenWithParallax(const glm::vec2 &world_pos,
                                            const glm::vec2 &scroll_factor) const;
        glm::vec2 screenToWorld(const glm::vec2 &screen_pos) const;

        // --- getters ---
        const glm::vec2 getPosition() const;
        std::optional<engine::utils::Rect> getLimitBounds() const;
        glm::vec2 getViewportSize() const;

        // --- setters ---
        void setPostion(const glm::vec2 &position);
        void setLimitBounds(const engine::utils::Rect &bounds);

    private:
        void clampPosition();
    };

} // namespace engine::render
