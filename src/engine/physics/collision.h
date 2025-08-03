#pragma once
#include "../utils/math.h"
#include "collider.h"

namespace engine::component {
    class ColliderComponent;
}

namespace engine::physics::collision {

    // 碰撞检测数据结构，避免重复计算
    struct CollisionData {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 center;
        float radius;
        engine::physics::ColliderType type;

        CollisionData(const engine::component::ColliderComponent &component);
    };

    /**
     * @brief 检查两个碰撞器组件是否重叠。
     * @param a 碰撞器组件 A。
     * @param b 碰撞器组件 B。
     */
    bool checkCollision(const engine::component::ColliderComponent &a,
                        const engine::component::ColliderComponent &b);

    /**
     * @brief 使用预计算数据检查碰撞
     * @param a 预计算数据 A。
     * @param b 预计算数据 B。
     */
    bool checkCollisionOptimized(const CollisionData &a, const CollisionData &b);

    /**
     * @brief 检查两个圆形是否重叠（使用距离平方优化）
     * @param a_center 圆形 A 的中心点。
     * @param a_radius 圆形 A 的半径。
     * @param b_center 圆形 B 的中心点。
     * @param b_radius 圆形 B 的半径。
     */
    bool checkCircleOverlap(const glm::vec2 &a_center, float a_radius, const glm::vec2 &b_center,
                            float b_radius);

    /**
     * @brief 检查AABB和圆形是否重叠
     * @param aabb_pos AABB 的位置。
     * @param aabb_size AABB 的大小。
     * @param circle_center 圆形的圆心。
     * @param circle_radius 圆形的半径。
     */
    bool checkAABBCircleOverlap(const glm::vec2 &aabb_pos, const glm::vec2 &aabb_size,
                                const glm::vec2 &circle_center, float circle_radius);

    /**
     * @brief 检查两个AABB是否重叠
     * @param a_pos a_AABB 的位置。
     * @param a_size a_AABB 的大小。
     * @param b_pos b_AABB 的位置。
     * @param b_size b_AABB 的大小。
     */
    bool checkAABBOverlap(const glm::vec2 &a_pos, const glm::vec2 &a_size, const glm::vec2 &b_pos,
                          const glm::vec2 &b_size);

    /**
     * @brief 检查两个矩形是否重叠
     * @param a 矩形 a
     * @param b 矩形 b
     */
    bool checkRectOverlap(const engine::utils::Rect &a, const engine::utils::Rect &b);

    /**
     * @brief 检查点是否在圆内（使用距离平方优化）
     * @param point 待检查点
     * @param center 圆心
     * @param radius 半径
     */
    bool checkPointInCircle(const glm::vec2 &point, const glm::vec2 &center, float radius);

} // namespace engine::physics::collision
