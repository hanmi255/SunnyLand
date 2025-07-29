#pragma once
#include <memory> // 用于 std::unique_ptr

struct SDL_Window;
struct SDL_Renderer;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::core {
    class Time;

    /**
     * @brief 游戏主程序，初始化SDL，并运行游戏循环
     */
    class GameApp final {
    private:
        SDL_Window *window_ = nullptr;
        SDL_Renderer *sdl_renderer_ = nullptr;
        bool is_running_ = false;

        // 引擎组件
        std::unique_ptr<engine::core::Time> time_;
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_;

    public:
        GameApp();
        ~GameApp();

        /**
         * @brief
         * 运行游戏应用程序，其中会调用init()，然后进入主循环，离开循环后自动调用close()。
         */
        void run();

        // 禁止拷贝和移动
        GameApp(const GameApp &) = delete;
        GameApp &operator=(const GameApp &) = delete;
        GameApp(GameApp &&) = delete;
        GameApp &operator=(GameApp &&) = delete;

    private:
        [[nodiscard]] bool init();
        void handleEvents();
        void update(float delta_time);
        void render();
        void close();

        // 模块初始化函数
        bool initSDL();
        bool initTime();
        bool initResourceManager();

        // 设置资源路径
        bool setupAssetPath();
    };
} // namespace engine::core