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
#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>

int main(int /* argc */, char* /* argv */[])
{
    spdlog::set_level(spdlog::level::off);

    engine::core::GameApp app;
    app.run();
    return 0;
}
