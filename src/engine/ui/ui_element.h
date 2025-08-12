#pragma once
#include "../utils/math.h"
#include <SDL3/SDL_rect.h>
#include <memory>
#include <vector>

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::ui {

    /**
     * @brief 所有UI元素的基类
     *
     * 定义了位置、大小、可见性、状态等通用属性。
     * 管理子元素的层次结构。
     * 提供事件处理、更新和渲染的虚方法。
     */
    class UIElement {
    protected:
        glm::vec2 position_;                               ///< @brief 相对于父元素的局部位置
        glm::vec2 size_;                                   ///< @brief 元素大小
        bool visible_ = true;                              ///< @brief 元素当前是否可见
        bool need_remove_ = false;                         ///< @brief 是否需要移除(延迟删除)

        UIElement* parent_ = nullptr;                      ///< @brief 指向父节点的非拥有指针
        std::vector<std::unique_ptr<UIElement>> children_; ///< @brief 子元素列表(容器)

    public:
        /**
         * @brief 构造UIElement
         * @param position 初始局部位置
         * @param size 初始大小
         */
        explicit UIElement(glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f});
        virtual ~UIElement() = default;

        // --- 核心逻辑 ---
        virtual bool handleInput(engine::core::Context &context);
        virtual void update(float delta_time, engine::core::Context &context);
        virtual void render(engine::core::Context &context);

        // --- 层次结构管理 ---
        void addChild(std::unique_ptr<UIElement> child);
        std::unique_ptr<UIElement> removeChild(UIElement* child_ptr);
        void removeAllChildren();

        // --- getters ---
        const glm::vec2 &getSize() const { return size_; }
        const glm::vec2 &getPosition() const { return position_; }
        bool isVisible() const { return visible_; }
        bool isNeedRemove() const { return need_remove_; }
        UIElement* getParent() const { return parent_; }
        const std::vector<std::unique_ptr<UIElement>> &getChildren() const { return children_; }

        // --- setters ---
        void setSize(glm::vec2 size) { size_ = std::move(size); }
        void setVisible(bool visible) { visible_ = visible; }
        void setParent(UIElement* parent) { parent_ = parent; }
        void setPosition(glm::vec2 position) { position_ = std::move(position); } // 相对于父节点
        void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }

        // --- 辅助方法 ---
        engine::utils::Rect getBounds() const;
        glm::vec2 getScreenPosition() const;
        bool isPointInside(const glm::vec2 &point) const;
        void cleanupChildren();

        // --- 禁用拷贝和移动语义 ---
        UIElement(const UIElement &) = delete;
        UIElement &operator=(const UIElement &) = delete;
        UIElement(UIElement &&) = delete;
        UIElement &operator=(UIElement &&) = delete;
    };

} // namespace engine::ui