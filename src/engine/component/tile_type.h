#pragma once
#include <cstdint>

namespace engine::component {
    /**
     * @brief 定义瓦片的类型，用于游戏逻辑（例如碰撞）。
     */
    enum class TileType : std::uint8_t {
        EMPTY,     ///< @brief 空白瓦片
        NORMAL,    ///< @brief 普通瓦片
        SOLID,     ///< @brief 静止可碰撞瓦片
        UNISOLID,  ///< @brief 单向静止可碰撞瓦片
        SLOPE_0_1, ///< @brief 斜坡瓦片，高度:左0   右1
        SLOPE_1_0, ///< @brief 斜坡瓦片，高度:左1   右0
        SLOPE_0_2, ///< @brief 斜坡瓦片，高度:左0   右1/2
        SLOPE_2_1, ///< @brief 斜坡瓦片，高度:左1/2 右1
        SLOPE_1_2, ///< @brief 斜坡瓦片，高度:左1   右1/2
        SLOPE_2_0, ///< @brief 斜坡瓦片，高度:左1/2 右0
        HAZARD,    ///< @brief 危险瓦片
        LADDER,    ///< @brief 梯子瓦片
    };
}; // namespace engine::component