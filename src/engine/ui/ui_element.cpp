#include "ui_element.h"
#include "../core/context.h"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <utility>

namespace engine::ui {

    UIElement::UIElement(glm::vec2 position, glm::vec2 size)
        : position_(std::move(position)), size_(std::move(size))
    {}

    bool UIElement::handleInput(engine::core::Context &context)
    {
        if (!visible_) return false;

        // 删除标记为需要移除的元素
        cleanupChildren();

        // 处理子元素的输入
        for (const auto &child : children_) {
            if (child->handleInput(context)) return true;
        }

        return false;
    }

    void UIElement::update(float delta_time, engine::core::Context &context)
    {
        if (!visible_) return;

        // 删除标记为需要移除的元素
        cleanupChildren();

        // 更新子元素
        for (const auto &child : children_) {
            child->update(delta_time, context);
        }
    }

    void UIElement::render(engine::core::Context &context)
    {
        if (!visible_) return;

        // 渲染子元素
        for (const auto &child : children_) {
            if (child) child->render(context);
        }
    }

    void UIElement::addChild(std::unique_ptr<UIElement> child)
    {
        if (child) {
            child->setParent(this); // 设置父节点
            children_.push_back(std::move(child));
        }
    }

    std::unique_ptr<UIElement> UIElement::removeChild(UIElement* child_ptr)
    {
        if (!child_ptr) return nullptr;

        // 使用 std::find_if 和 lambda 表达式查找要移除的子元素
        auto it = std::find_if(
            children_.begin(), children_.end(),
            [child_ptr](const std::unique_ptr<UIElement> &p) { return p.get() == child_ptr; });

        if (it != children_.end()) {
            std::unique_ptr<UIElement> removed_child = std::move(*it);
            children_.erase(it);
            removed_child->setParent(nullptr); // 清除父节点
            return removed_child;
        }
        return nullptr;
    }

    void UIElement::removeAllChildren()
    {
        for (auto &child : children_) {
            child->setParent(nullptr); // 清除父节点
        }
        children_.clear();
    }

    glm::vec2 UIElement::getScreenPosition() const
    {
        if (parent_) {
            return parent_->getScreenPosition() + position_;
        }
        return position_; // 根元素的位置已经是相对屏幕的绝对位置
    }

    engine::utils::Rect UIElement::getBounds() const
    {
        auto abs_pos = getScreenPosition();
        return engine::utils::Rect(abs_pos, size_);
    }

    bool UIElement::isPointInside(const glm::vec2 &point) const
    {
        auto bounds = getBounds();
        return (point.x >= bounds.position.x && point.x < (bounds.position.x + bounds.size.x) &&
                point.y >= bounds.position.y && point.y < (bounds.position.y + bounds.size.y));
    }

    void UIElement::cleanupChildren()
    {
        children_.erase(std::remove_if(children_.begin(), children_.end(),
                                       [](const std::unique_ptr<UIElement> &child) {
                                           return !child || child->isNeedRemove();
                                       }),
                        children_.end());
    }

} // namespace engine::ui