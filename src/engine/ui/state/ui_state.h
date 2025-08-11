#pragma once
#include <memory>

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::ui {
    class UIInteractive;
} // namespace engine::ui

namespace engine::ui::state {

    /**
     * @brief 可交互UI元素在特定状态下的行为接口。
     *
     * 该接口定义了所有具体UI状态必须实现的通用操作，
     * 例如处理输入事件、更新状态逻辑以及确定视觉表现。
     */
    class UIState {
        friend class engine::ui::UIInteractive;

    protected:
        engine::ui::UIInteractive* owner_ = nullptr; ///< @brief 指向父节点

    public:
        UIState(engine::ui::UIInteractive* owner) : owner_(owner) {}
        virtual ~UIState() = default;

        // 禁止拷贝和移动语义
        UIState(const UIState &) = delete;
        UIState &operator=(const UIState &) = delete;
        UIState(UIState &&) = delete;
        UIState &operator=(UIState &&) = delete;

    protected:
        // 核心逻辑
        virtual void enter() {}
        virtual std::unique_ptr<UIState> handleInput(engine::core::Context &context) = 0;
    };

} // namespace engine::ui::state