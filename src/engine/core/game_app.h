#pragma once
#include <memory>

struct SDL_Window;
struct SDL_Renderer;

namespace engine::core {
class Time;

    /**
     * @brief 游戏主程序，初始化SDL，并运行游戏循环
     */
    class GameApp final { // final 表示不能被继承
    private:
        SDL_Window *window_ = nullptr;
        SDL_Renderer *sdl_renderer_ = nullptr;
        bool is_running_ = false;

        // 引擎组件
        std::unique_ptr<engine::core::Time> time_;

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
        [[nodiscard]] bool init(); // nodiscard 表示该函数返回值不应该被忽略
        void handleEvents();
        void update(float delta_time);
        void render();
        void close();
    };
} // namespace engine::core