#pragma once
#include <memory>
#include <vector>

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::scene {
    class Scene;

    /**
     * @brief 管理游戏中的场景栈，处理场景切换和生命周期。
     */
    class SceneManager final {
    private:
        engine::core::Context &context_;                   ///< @brief 上下文引用
        std::vector<std::unique_ptr<Scene>> scenes_stack_; ///< @brief 场景栈

        enum class PendingAction : std::uint8_t {
            NONE,
            POP,
            PUSH,
            REPLACE
        };                                                   ///< @brief 待处理的操作
        PendingAction pending_action_ = PendingAction::NONE; ///< @brief 默认待处理的操作
        std::unique_ptr<Scene> pending_scene_;               ///< @brief 待处理的场景

    public:
        explicit SceneManager(engine::core::Context &context);
        ~SceneManager();

        // 禁止拷贝和移动语义
        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;
        SceneManager(SceneManager &&) = delete;
        SceneManager &operator=(SceneManager &&) = delete;

        // 延时切换场景（发出请求）
        void requestPushScene(std::unique_ptr<Scene> &&scene);
        void requestPopScene();
        void requestReplaceScene(std::unique_ptr<Scene> &&scene);

        // --- getters ---
        [[nodiscard]] Scene* getCurrentScene() const;
        [[nodiscard]] engine::core::Context &getContext() const { return context_; }
        [[nodiscard]] bool isEmpty() const { return scenes_stack_.empty(); }
        [[nodiscard]] size_t getSceneCount() const { return scenes_stack_.size(); }

        // 核心逻辑
        void update(float delta_time);
        void render();
        void handleInput();
        void clean();

    private:
        void processPendingActions();
        void pushScene(std::unique_ptr<Scene> &&scene);
        void popScene();
        void replaceScene(std::unique_ptr<Scene> &&scene);
        void resetPendingAction();
    };
} // namespace engine::scene
