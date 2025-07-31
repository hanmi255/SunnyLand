/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 18:51:07
 * @LastEditTime: 2025-07-31 09:56:55
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\utils\math.h
 * @技术宅拯救世界！！！
 */
#pragma once
#include <glm/glm.hpp>

namespace engine::utils {

    struct Rect {
        glm::vec2 position;
        glm::vec2 size;
    };

    struct FColor {
        float r;
        float g;
        float b;
        float a;
    };

} // namespace engine::utils
