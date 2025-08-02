#pragma once
#include "../render/sprite.h"
#include "component.h"
#include <glm/vec2.hpp>
#include <string>

namespace engine::component {
    class TransformComponent;

    /**
     * @brief 在背景中渲染可滚动纹理的组件，以创建视差效果。
     *
     * 该组件根据相机的位置和滚动因子来移动纹理。
     */
    class ParallaxComponent final : public Component {
        friend class engine::object::GameObject;

    private:
        TransformComponent* transform_component_ = nullptr; ///<@brief 变换组件

        engine::render::Sprite sprite_;                     ///<@brief 精灵对象
        glm::vec2 scroll_factor_; ///<@brief 滚动因子(0=静止, 1=随相机移动, <1=比相机慢)
        glm::bvec2 repeat_xy_;    ///<@brief 是否沿着x和y轴重复
        bool is_visible_ = true;  ///<@brief 是否可见

    public:
        /**
         * @brief 构造函数
         * @param texture_id 背景纹理的资源 ID。
         * @param scroll_factor 控制背景相对于相机移动速度的因子。
         *                      (0, 0) 表示完全静止。
         *                      (1, 1) 表示与相机完全同步移动。
         *                      (0.5, 0.5) 表示以相机一半的速度移动。
         * @param repeat_xy 是否沿着X Y轴重复
         */
        ParallaxComponent(const std::string &texture_id, const glm::vec2 &scroll_factor,
                          const glm::bvec2 &repeat_xy);

        // --- getters ---
        const engine::render::Sprite &getSprite() const { return sprite_; }
        const glm::vec2 &getScrollFactor() const { return scroll_factor_; }
        const glm::bvec2 &getRepeatXY() const { return repeat_xy_; }
        bool isVisible() const { return is_visible_; }

        // --- setters ---
        void setSprite(const engine::render::Sprite &sprite) { sprite_ = sprite; }
        void setScrollFactor(const glm::vec2 &factor) { scroll_factor_ = factor; }
        void setRepeatXY(const glm::bvec2 &repeat_xy) { repeat_xy_ = repeat_xy; }
        void setVisible(bool visible) { is_visible_ = visible; }

    private:
        // 核心逻辑
        void init() override;
        void update(float, engine::core::Context &) override {}
        void render(engine::core::Context &context) override;
    };
} // namespace engine::component
