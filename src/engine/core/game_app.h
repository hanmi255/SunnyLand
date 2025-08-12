#pragma once
#include <functional>
#include <memory> // 用于 std::unique_ptr

struct SDL_Window;
struct SDL_Renderer;

namespace engine::audio {
    class AudioPlayer;
} // namespace engine::audio

namespace engine::input {
    class InputManager;
} // namespace engine::input

namespace engine::physics {
    class PhysicsEngine;
} // namespace engine::physics

namespace engine::render {
    class Camera;
    class Renderer;
    class TextRenderer;
} // namespace engine::render

namespace engine::resource {
    class ResourceManager;
} // namespace engine::resource

namespace engine::scene {
    class SceneManager;
}

namespace engine::core {
    class Config;
    class Context;
    class GameState;
    class Time;

    /**
     * @brief 游戏主程序，初始化SDL，并运行游戏循环
     */
    class GameApp final {
    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* sdl_renderer_ = nullptr;
        bool is_running_ = false;

        std::function<void(engine::scene::SceneManager &)> scene_setup_func_;

        // 引擎组件
        std::unique_ptr<engine::core::Config> config_;
        std::unique_ptr<engine::core::Context> context_;
        std::unique_ptr<engine::core::GameState> game_state_;
        std::unique_ptr<engine::core::Time> time_;
        std::unique_ptr<engine::input::InputManager> input_manager_;
        std::unique_ptr<engine::audio::AudioPlayer> audio_player_;
        std::unique_ptr<engine::physics::PhysicsEngine> physics_engine_;
        std::unique_ptr<engine::render::Camera> camera_;
        std::unique_ptr<engine::render::Renderer> renderer_;
        std::unique_ptr<engine::render::TextRenderer> text_renderer_;
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_;
        std::unique_ptr<engine::scene::SceneManager> scene_manager_;

    public:
        GameApp();
        ~GameApp();

        /**
         * @brief
         * 运行游戏应用程序，其中会调用init()，然后进入主循环，离开循环后自动调用close()。
         */
        void run();

        /**
         * @brief 注册用于设置初始游戏场景的函数。
         *        这个函数将在 SceneManager 初始化后被调用。
         * @param func 一个接收 SceneManager 引用的函数对象。
         */
        void registerSceneSetup(std::function<void(engine::scene::SceneManager &)> func);

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
        [[nodiscard]] bool initConfig();
        [[nodiscard]] bool initSDL();
        [[nodiscard]] bool initTime();
        [[nodiscard]] bool initResourceManager();
        [[nodiscard]] bool initAudioPlayer();
        [[nodiscard]] bool initRenderer();
        [[nodiscard]] bool initCamera();
        [[nodiscard]] bool initTextRenderer();
        [[nodiscard]] bool initInputManager();
        [[nodiscard]] bool initPhysicsEngine();
        [[nodiscard]] bool initGameState();
        [[nodiscard]] bool initContext();
        [[nodiscard]] bool initSceneManager();

        // 设置资源路径
        bool setupAssetPath();
    };
} // namespace engine::core
