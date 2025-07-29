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
#include "time.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::core {
    GameApp::GameApp() { time_ = std::make_unique<Time>(); }

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

        time_->setTargetFps(144);   // 模拟帧率

        while (is_running_) {
            time_->update();
            float delta_time = time_->getDeltaTime();

            handleEvents();
            update(delta_time);
            render();

            spdlog::info("delta_time: {:.2f}", delta_time);
        }

        close();
    }

    bool GameApp::init() {
        spdlog::trace("GameApp 正在初始化...");
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            spdlog::error("SDL_Init failed: {}", SDL_GetError());
            return false;
        }

        window_ =
            SDL_CreateWindow("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE);
        if (window_ == nullptr) {
            spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
            return false;
        }

        sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (sdl_renderer_ == nullptr) {
            spdlog::error("SDL_CreateRenderer failed: {}", SDL_GetError());
            return false;
        }

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
} // namespace engine::core
