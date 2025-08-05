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
        glm::vec2 position_ = {0.0f, 0.0f}; ///< @brief 位置
        glm::vec2 scale_ = {1.0f, 1.0f};    ///< @brief 缩放
        float rotation_ = 0.0f;             ///< @brief 旋转 角度制 单位：度

    public:
        /**
         * @brief 构造函数
         * @param position 位置
         * @param scale 缩放
         * @param rotation 旋转
         */
        TransformComponent(glm::vec2 position = {0.0f, 0.0f},
                           glm::vec2 scale = {1.0f, 1.0f},
                           float rotation = 0.0f)
            : position_(position), scale_(scale), rotation_(rotation){}

        // 禁止拷贝和移动语义
        TransformComponent(const TransformComponent &) = delete;
        TransformComponent &operator=(const TransformComponent &) = delete;
        TransformComponent(TransformComponent &&) = delete;
        TransformComponent &operator=(TransformComponent &&) = delete;

        // --- getters ---
        const glm::vec2 &getPosition() const { return position_; }
        const glm::vec2 &getScale() const { return scale_; }
        const float getRotation() const { return rotation_; }

        // --- setters ---
        void setPosition(const glm::vec2 &position) { position_ = position; }
        void setScale(const glm::vec2 &scale);
        void setRotation(float rotation) { rotation_ = rotation; }

        void translate(const glm::vec2 &offset) { position_ += offset; }

    private:
        void update(float, engine::core::Context &) override {}
    };
} // namespace engine::component
