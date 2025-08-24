#pragma once

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

namespace engine::core {
    class GameState;

    /**
     * @brief 持有对核心引擎模块引用的上下文对象。
     *
     * 用于简化依赖注入，传递Context即可获取引擎的各个模块。
     */
    class Context final {
    private:
        engine::audio::AudioPlayer &audio_player_;
        engine::core::GameState &game_state_;
        engine::input::InputManager &input_manager_;
        engine::render::Camera &camera_;
        engine::render::Renderer &renderer_;
        engine::render::TextRenderer &text_renderer_;
        engine::resource::ResourceManager &resource_manager_;
        engine::physics::PhysicsEngine &physics_engine_;

    public:
        /**
         * @brief 构造函数。
         * @param input_manager 对 InputManager 实例的引用。
         * @param camera 对 Camera 实例的引用。
         * @param renderer 对 Renderer 实例的引用。
         * @param resource_manager 对 ResourceManager 实例的引用。
         * @param physics_engine 对 PhysicsEngine 实例的引用。
         * @param audio_player 对 AudioPlayer 实例的引用。
         */
        Context(engine::input::InputManager &input_manager, engine::render::Camera &camera,
                engine::render::Renderer &renderer, engine::render::TextRenderer &text_renderer,
                engine::resource::ResourceManager &resource_manager,
                engine::physics::PhysicsEngine &physics_engine,
                engine::audio::AudioPlayer &audio_player,
                engine::core::GameState &game_state);

        // 禁止拷贝和移动语义
        Context(const Context &) = delete;
        Context &operator=(const Context &) = delete;
        Context(Context &&) = delete;
        Context &operator=(const Context &&) = delete;

        // -- getters ---
        [[nodiscard]] engine::input::InputManager &getInputManager() const { return input_manager_; }
        [[nodiscard]] engine::render::Camera &getCamera() const { return camera_; }
        [[nodiscard]] engine::render::Renderer &getRenderer() const { return renderer_; }
        [[nodiscard]] engine::render::TextRenderer &getTextRenderer() const { return text_renderer_; }
        [[nodiscard]] engine::resource::ResourceManager &getResourceManager() const { return resource_manager_; }
        [[nodiscard]] engine::physics::PhysicsEngine &getPhysicsEngine() const { return physics_engine_; }
        [[nodiscard]] engine::audio::AudioPlayer &getAudioPlayer() const { return audio_player_; }
        [[nodiscard]] engine::core::GameState &getGameState() const { return game_state_; }
    };
} // namespace engine::core
