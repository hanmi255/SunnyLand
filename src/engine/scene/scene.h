#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace engine::ui {
    class UIManager;
} // namespace engine::ui

namespace engine::scene {
    class SceneManager;

    /**
     * @brief 场景基类，负责管理场景中的游戏对象和场景生命周期。
     *
     * 包含一组游戏对象，并提供更新、渲染、处理输入和清理的接口。
     * 派生类应实现具体的场景逻辑。
     */
    class Scene {
    protected:
        std::string name_;                                  ///< @brief 场景名称
        engine::core::Context &context_;                    ///< @brief 上下文引用
        engine::scene::SceneManager &scene_manager_;        ///< @brief 场景管理器引用
        std::unique_ptr<engine::ui::UIManager> ui_manager_; ///< @brief UI管理器引用
        bool is_initialized_ = false;                       ///< @brief 是否已初始化
        std::vector<std::unique_ptr<engine::object::GameObject>>
            game_objects_;                                  ///< @brief 场景中的游戏对象列表
        std::vector<std::unique_ptr<engine::object::GameObject>>
            pending_additions_;                             ///< @brief 待处理的游戏对象列表

    public:
        /**
         * @brief 构造函数。
         *
         * @param name 场景的名称。
         * @param context 场景上下文。
         * @param scene_manager 场景管理器。
         */
        Scene(const std::string_view &name, engine::core::Context &context,
              engine::scene::SceneManager &scene_manager);
        virtual ~Scene();

        // 禁止拷贝和移动语义
        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;
        Scene(Scene &&) = delete;
        Scene &operator=(Scene &&) = delete;

        // 核心逻辑
        virtual void init();
        virtual void update(float delta_time);
        virtual void render();
        virtual void handleInput();
        virtual void clean();

        virtual void addGameObject(std::unique_ptr<engine::object::GameObject> &&object);
        virtual void safelyAddGameObject(std::unique_ptr<engine::object::GameObject> &&object);
        virtual void removeGameObject(engine::object::GameObject* object_ptr);
        virtual void safelyRemoveGameObject(engine::object::GameObject* object_ptr);

        engine::object::GameObject* findGameObjectByName(const std::string &name) const;

        // --- getters ---
        const std::string &getName() const { return name_; }
        engine::core::Context &getContext() const { return context_; }
        engine::scene::SceneManager &getSceneManager() const { return scene_manager_; }
        bool isInitialized() const { return is_initialized_; }
        const std::vector<std::unique_ptr<engine::object::GameObject>> &getGameObjects() const
        {
            return game_objects_;
        }

        // --- setters ---
        void setName(const std::string_view &name) { name_ = name; }
        void setInitialized(bool initialized) { is_initialized_ = initialized; }

    protected:
        void processPendingActions();
    };

} // namespace engine::scene
