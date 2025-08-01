/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 14:34:14
 * @LastEditTime: 2025-07-31 12:39:15
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\core\game_app.cpp
 * @技术宅拯救世界！！！
 */
#include "game_app.h"
#include "../component/sprite_component.h"
#include "../component/transform_component.h"
#include "../input/input_manager.h"
#include "../object/game_object.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "config.h"
#include "context.h"
#include "time.h"
#include <SDL3/SDL.h>
#include <filesystem> // 用于 std::filesystem
#include <spdlog/spdlog.h>

namespace engine::core {
    engine::object::GameObject go("test");

    GameApp::GameApp() = default;

    GameApp::~GameApp()
    {
        if (is_running_) {
            spdlog::warn("GameApp 仍在运行...尝试关闭");
            close();
        }
    }

    void GameApp::run()
    {
        if (!init()) {
            spdlog::error("GameApp 初始化失败");
            return;
        }

        while (is_running_) {
            time_->update();
            double delta_time = time_->getDeltaTime();

            input_manager_->update();

            handleEvents();
            update(delta_time);
            render();

            // spdlog::info("delta_time: {:.2f}", delta_time);
        }

        close();
    }

    bool GameApp::init()
    {
        spdlog::trace("GameApp 正在初始化...");

        setupAssetPath();

        if (!initConfig()) return false;
        if (!initSDL()) return false;
        if (!initTime()) return false;
        if (!initResourceManager()) return false;
        if (!initRenderer()) return false;
        if (!initCamera()) return false;
        if (!initInputManager()) return false;

        if (!initContext()) return false;

        testResourceManager();

        is_running_ = true;
        spdlog::info("GameApp 初始化成功");
        testGameObject();
        return true;
    }

    void GameApp::handleEvents()
    {
        if (input_manager_->shouldQuit()) {
            spdlog::trace("GameApp 收到 InputManager 的退出信号");
            is_running_ = false;
            return;
        }

        testInputManager();
    }

    void GameApp::update(double /* delta_time */)
    {
        testCamera();
    }

    void GameApp::render()
    {
        renderer_->clearScreen();

        testRenderer();
        go.render(*context_);

        renderer_->present();
    }

    void GameApp::close()
    {
        spdlog::trace("GameApp 正在关闭...");
        if (sdl_renderer_ != nullptr) {
            SDL_DestroyRenderer(sdl_renderer_);
            sdl_renderer_ = nullptr;
        }
        if (window_ != nullptr) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
        is_running_ = false;
    }

    bool GameApp::initConfig()
    {
        try {
            config_ = std::make_unique<engine::core::Config>("assets/config/config.json");
        } catch (const std::exception &e) {
            spdlog::error("Config 初始化失败: {}", e.what());
            return false;
        }
        spdlog::trace("Config 初始化成功");
        return true;
    }

    bool GameApp::initSDL()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            spdlog::error("SDL 初始化失败: {}", SDL_GetError());
            return false;
        }

        window_ = SDL_CreateWindow(config_->getWindowTitle().c_str(), config_->getWindowWidth(),
                                   config_->getWindowHeight(), SDL_WINDOW_RESIZABLE);
        if (window_ == nullptr) {
            spdlog::error("SDL 创建窗口失败: {}", SDL_GetError());
            return false;
        }

        sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (sdl_renderer_ == nullptr) {
            spdlog::error("SDL 创建渲染器失败: {}", SDL_GetError());
            return false;
        }

        // 设置 VSync
        // (注意: VSync开启时，驱动程序会尝试将帧率限制到显示器刷新率，有可能会覆盖我们手动设置的
        // target_fps)
        int vsync_mode = config_->isVSyncEnabled() ? SDL_RENDERER_VSYNC_ADAPTIVE
                                                   : SDL_RENDERER_VSYNC_DISABLED;
        SDL_SetRenderVSync(sdl_renderer_, vsync_mode);
        spdlog::trace("VSync 设置为: {}", config_->isVSyncEnabled() ? "Enabled" : "Disabled");

        // 设置逻辑分辨率（针对像素风）
        SDL_SetRenderLogicalPresentation(sdl_renderer_, config_->getWindowWidth() / 2,
                                         config_->getWindowHeight() / 2,
                                         SDL_LOGICAL_PRESENTATION_LETTERBOX);

        spdlog::trace("SDL 初始化成功");
        return true;
    }

    bool GameApp::initTime()
    {
        try {
            time_ = std::make_unique<engine::core::Time>();
        } catch (const std::exception &e) {
            spdlog::error("Time 初始化失败: {}", e.what());
            return false;
        }
        time_->setTargetFps(config_->getTargetFps());
        spdlog::trace("Time 初始化成功");
        return true;
    }

    bool GameApp::initResourceManager()
    {
        try {
            resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);
        } catch (const std::exception &e) {
            spdlog::error("ResourceManager 初始化失败: {}", e.what());
            return false;
        }
        spdlog::trace("ResourceManager 初始化成功");
        return true;
    }

    bool GameApp::initRenderer()
    {
        try {
            renderer_ =
                std::make_unique<engine::render::Renderer>(sdl_renderer_, resource_manager_.get());
        } catch (const std::exception &e) {
            spdlog::error("Renderer 初始化失败: {}", e.what());
            return false;
        }
        spdlog::trace("Renderer 初始化成功");
        return true;
    }

    bool GameApp::initCamera()
    {
        try {
            camera_ = std::make_unique<engine::render::Camera>(
                glm::vec2(config_->getWindowWidth() / 2, config_->getWindowHeight() / 2));
        } catch (const std::exception &e) {
            spdlog::error("Camera 初始化失败: {}", e.what());
            return false;
        }

        spdlog::trace("Camera 初始化成功");
        return true;
    }

    bool GameApp::initInputManager()
    {
        try {
            input_manager_ =
                std::make_unique<engine::input::InputManager>(sdl_renderer_, config_.get());
        } catch (const std::exception &e) {
            spdlog::error("InputManager 初始化失败: {}", e.what());
            return false;
        }
        spdlog::info("InputManager 初始化成功");
        return true;
    }

    bool GameApp::initContext()
    {
        try {
            context_ = std::make_unique<engine::core::Context>(*input_manager_, *camera_,
                                                               *renderer_, *resource_manager_);
        } catch (const std::exception &e) {
            spdlog::error("Context 初始化失败: {}", e.what());
            return false;
        }
        spdlog::info("Context 初始化成功");
        return true;
    }

    bool GameApp::setupAssetPath()
    {
        // 尝试设置资源目录，以便能找到assets文件夹
        std::filesystem::path current_path = std::filesystem::current_path();
        std::filesystem::path assets_path = current_path.parent_path().parent_path() / "assets";
        if (std::filesystem::exists(assets_path)) {
            std::filesystem::current_path(assets_path.parent_path());
            spdlog::info("设置工作目录到: {}", std::filesystem::current_path().string());
            return true;
        } else {
            spdlog::warn("未找到 assets 目录，使用默认工作目录");
            return false;
        }
    }

    void GameApp::testResourceManager()
    {
        resource_manager_->getTexture("assets/textures/Actors/eagle-attack.png");
        resource_manager_->getFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
        resource_manager_->getSound("assets/audio/button_click.wav");

        resource_manager_->unloadTexture("assets/textures/Actors/eagle-attack.png");
        resource_manager_->unloadFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
        resource_manager_->unloadSound("assets/audio/button_click.wav");
    }

    void GameApp::testRenderer()
    {
        engine::render::Sprite sprite_world("assets/textures/Actors/frog.png");
        engine::render::Sprite sprite_ui("assets/textures/UI/buttons/Start1.png");
        engine::render::Sprite sprite_parallax("assets/textures/Layers/back.png");

        static float rotation = 0.0f;
        rotation += 0.1f;

        // 注意渲染顺序
        renderer_->drawParallax(*camera_, sprite_parallax, glm::vec2(100, 100),
                                glm::vec2(0.5f, 0.5f), glm::bvec2(true, false));
        renderer_->drawSprite(*camera_, sprite_world, glm::vec2(200, 200), glm::vec2(1.0f, 1.0f),
                              rotation);
        renderer_->drawUISprite(sprite_ui, glm::vec2(100, 100));
    }

    void GameApp::testCamera()
    {
        auto key_state = SDL_GetKeyboardState(nullptr);
        if (key_state[SDL_SCANCODE_UP]) camera_->move(glm::vec2(0, -1));
        if (key_state[SDL_SCANCODE_DOWN]) camera_->move(glm::vec2(0, 1));
        if (key_state[SDL_SCANCODE_LEFT]) camera_->move(glm::vec2(-1, 0));
        if (key_state[SDL_SCANCODE_RIGHT]) camera_->move(glm::vec2(1, 0));
    }

    void GameApp::testInputManager()
    {
        std::vector<std::string> actions = {"move_up",    "move_down",      "move_left",
                                            "move_right", "jump",           "attack",
                                            "pause",      "MouseLeftClick", "MouseRightClick"};

        for (const auto &action : actions) {
            if (input_manager_->isActionJustPressed(action)) {
                spdlog::info(" {} 按下 ", action);
            }
            if (input_manager_->isActionJustReleased(action)) {
                spdlog::info(" {} 抬起 ", action);
            }
            if (input_manager_->isActionHeldDown(action)) {
                spdlog::info(" {} 按下中 ", action);
            }
        }
    }
    void GameApp::testGameObject()
    {
        go.addComponent<engine::component::TransformComponent>(glm::vec2(100, 100));
        go.addComponent<engine::component::SpriteComponent>("assets/textures/Props/big-crate.png",
                                                            *resource_manager_,
                                                            engine::utils::Alignment::CENTER);
    }
} // namespace engine::core
