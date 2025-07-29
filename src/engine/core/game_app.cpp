/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 14:34:14
 * @LastEditTime: 2025-07-29 15:22:33
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\core\game_app.cpp
 * @技术宅拯救世界！！！
 */
#include "game_app.h"
#include "../resource/resource_manager.h"
#include "time.h"
#include <SDL3/SDL.h>
#include <filesystem> // 用于 std::filesystem
#include <spdlog/spdlog.h>

namespace engine::core {
    GameApp::GameApp() = default;

    GameApp::~GameApp() {
        if (is_running_) {
            spdlog::warn("GameApp 仍在运行...尝试关闭");
            close();
        }
    }

    void GameApp::run() {
        if (!init()) {
            spdlog::error("GameApp 初始化失败");
            return;
        }

        time_->setTargetFps(144); // 模拟帧率

        while (is_running_) {
            time_->update();
            float delta_time = time_->getDeltaTime();

            handleEvents();
            update(delta_time);
            render();

            // spdlog::info("delta_time: {:.2f}", delta_time);
        }

        close();
    }

    bool GameApp::init() {
        spdlog::trace("GameApp 正在初始化...");

        setupAssetPath();

        if (!initSDL())
            return false;
        if (!initTime())
            return false;
        if (!initResourceManager())
            return false;

        is_running_ = true;
        spdlog::info("GameApp 初始化成功");
        return true;
    }

    void GameApp::handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                is_running_ = false;
            }
        }
    }

    void GameApp::update(float delta_time) {}

    void GameApp::render() {}

    void GameApp::close() {
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

    bool GameApp::initSDL() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            spdlog::error("SDL 初始化失败: {}", SDL_GetError());
            return false;
        }

        window_ = SDL_CreateWindow("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE);
        if (window_ == nullptr) {
            spdlog::error("SDL 创建窗口失败: {}", SDL_GetError());
            return false;
        }

        sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (sdl_renderer_ == nullptr) {
            spdlog::error("SDL 创建渲染器失败: {}", SDL_GetError());
            return false;
        }

        spdlog::trace("SDL 初始化成功");
        return true;
    }

    bool GameApp::initTime() {
        try {
            time_ = std::make_unique<Time>();
        } catch (const std::exception &e) {
            spdlog::error("Time 初始化失败: {}", e.what());
            return false;
        }
        spdlog::trace("Time 初始化成功");
        return true;
    }

    bool GameApp::initResourceManager() {
        try {
            resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);

        } catch (const std::exception &e) {
            spdlog::error("ResourceManager 初始化失败: {}", e.what());
            return false;
        }
        spdlog::trace("ResourceManager 初始化成功");
        return true;
    }

    bool GameApp::setupAssetPath() {
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
} // namespace engine::core