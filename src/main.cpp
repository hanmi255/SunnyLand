/*** 
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 12:58:45
 * @LastEditTime: 2025-07-29 13:49:08
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description: 
 * @FilePath: \SunnyLand\src\main.cpp
 * @技术宅拯救世界！！！
 */

#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>

int main(int /* argc */, char * /* argv */[]) {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Hello, SunnyLand!");
    spdlog::error("Some error message with arg: {}", 1);
    return 0;
}