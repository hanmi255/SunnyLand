/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 12:58:45
 * @LastEditTime: 2025-07-29 14:55:03
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\main.cpp
 * @技术宅拯救世界！！！
 */
#include "engine/core/game_app.h"
#include "engine/scene/scene_manager.h"
#include "game/scene/title_scene.h"
#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>
void setupInitialScene(engine::scene::SceneManager &scene_manager)
{
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto title_scene =
        std::make_unique<game::scene::TitleScene>(scene_manager.getContext(), scene_manager);
    scene_manager.requestPushScene(std::move(title_scene));
}
int main(int /* argc */, char* /* argv */[])
{
    spdlog::set_level(spdlog::level::off);

    engine::core::GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}
