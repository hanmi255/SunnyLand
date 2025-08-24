#pragma once
#include <glm/vec2.hpp>
#include <memory>

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::ui {
    class UIElement;
    class UIPanel; // UIPanel 将作为根元素
} // namespace engine::ui

namespace engine::ui {

    /**
     * @brief 管理特定场景中的UI元素集合。
     *
     * 负责UI元素的生命周期管理（通过根元素）、渲染调用和输入事件分发。
     * 每个需要UI的场景（如菜单、游戏HUD）应该拥有一个UIManager实例。
     */
    class UIManager final {
    private:
        std::unique_ptr<UIPanel> root_element_; ///< @brief 一个UIPanel作为根节点(UI元素)

    public:
        UIManager(); ///< @brief 构造函数将创建默认的根节点。
        ~UIManager();

        [[nodiscard]] bool init(const glm::vec2 &window_size);
        void addElement(std::unique_ptr<UIElement> element);
        [[nodiscard]] UIPanel* getRootElement() const;
        void clearElements();

        // --- 核心逻辑 ---
        bool handleInput(engine::core::Context &);
        void update(float delta_time, engine::core::Context &);
        void render(engine::core::Context &);

        // 禁止拷贝和移动语义
        UIManager(const UIManager &) = delete;
        UIManager &operator=(const UIManager &) = delete;
        UIManager(UIManager &&) = delete;
        UIManager &operator=(UIManager &&) = delete;
    };

} // namespace engine::ui