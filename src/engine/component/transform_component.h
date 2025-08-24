#pragma once
#include "component.h"
#include <glm/vec2.hpp>

namespace engine::component {
    /**
     * @class TransformComponent
     * @brief 管理 GameObject 的位置、旋转和缩放。
     */
    class TransformComponent final : public engine::component::Component {
        friend class engine::object::GameObject;

    private:
        glm::vec2 position_ = {0.0F, 0.0F}; ///< @brief 位置
        glm::vec2 scale_ = {1.0F, 1.0F};    ///< @brief 缩放
        float rotation_ = 0.0F;             ///< @brief 旋转 角度制 单位：度

    public:
        /**
         * @brief 构造函数
         * @param position 位置
         * @param scale 缩放
         * @param rotation 旋转
         */
        explicit TransformComponent(glm::vec2 position = {0.0F, 0.0F},
                                    glm::vec2 scale = {1.0F, 1.0F}, float rotation = 0.0F)
            : position_(position), scale_(scale), rotation_(rotation)
        {}

        // 禁止拷贝和移动语义
        TransformComponent(const TransformComponent &) = delete;
        TransformComponent &operator=(const TransformComponent &) = delete;
        TransformComponent(TransformComponent &&) = delete;
        TransformComponent &operator=(TransformComponent &&) = delete;

        // --- getters ---
        [[nodiscard]] const glm::vec2 &getPosition() const { return position_; }
        [[nodiscard]] const glm::vec2 &getScale() const { return scale_; }
        [[nodiscard]] float getRotation() const { return rotation_; }

        // --- setters ---
        void setPosition(glm::vec2 position) { position_ = position; }
        void setScale(glm::vec2 scale);
        void setRotation(float rotation) { rotation_ = rotation; }

        void translate(const glm::vec2 &offset) { position_ += offset; }

    private:
        void update(float /*unused*/, engine::core::Context & /*unused*/) override {}
    };
} // namespace engine::component
