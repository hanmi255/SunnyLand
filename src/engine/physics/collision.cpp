#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include "collision.h"
#include "../component/collider_component.h"
#include "../component/transform_component.h"
#include <glm/gtx/norm.hpp> // 用于 glm::length2()

namespace engine::physics::collision {

    CollisionData::CollisionData(const engine::component::ColliderComponent &component)
    {
        const auto* collider = component.getCollider();
        auto* transform = component.getTransform();

        size = collider->getAABBSize() * transform->getScale();
        position = transform->getPosition() + component.getOffset();
        center = position + 0.5F * size;
        radius = 0.5F * size.x; // 假设圆形半径为宽度的一半
        type = collider->getType();
    }

    bool checkCollision(const engine::component::ColliderComponent &a,
                        const engine::component::ColliderComponent &b)
    {
        // 创建预计算数据
        CollisionData data_a(a);
        CollisionData data_b(b);

        return checkCollisionOptimized(data_a, data_b);
    }

    bool checkCollisionOptimized(const CollisionData &a, const CollisionData &b)
    {
        // 早期AABB检测 - 如果AABB不重叠，直接返回false
        if (!checkAABBOverlap(a.position, a.size, b.position, b.size)) {
            return false;
        }

        // 根据碰撞器类型进行精确检测
        if (a.type == engine::physics::ColliderType::AABB &&
            b.type == engine::physics::ColliderType::AABB) {
            return true; // AABB已经检测过了
        }

        if (a.type == engine::physics::ColliderType::CIRCLE &&
            b.type == engine::physics::ColliderType::CIRCLE) {
            return checkCircleOverlap(a.center, a.radius, b.center, b.radius);
        }

        // AABB vs Circle 或 Circle vs AABB
        const CollisionData* aabb_data = (a.type == engine::physics::ColliderType::AABB) ? &a : &b;
        const CollisionData* circle_data = (a.type == engine::physics::ColliderType::CIRCLE) ? &a
                                                                                             : &b;

        return checkAABBCircleOverlap(aabb_data->position, aabb_data->size, circle_data->center,
                                      circle_data->radius);
    }

    bool checkCircleOverlap(const glm::vec2 &a_center, float a_radius, const glm::vec2 &b_center,
                            float b_radius)
    {
        // 使用距离平方比较，避免开方运算
        float distance_squared = glm::length2(a_center - b_center);
        float radius_sum = a_radius + b_radius;
        return distance_squared < (radius_sum * radius_sum);
    }

    bool checkAABBCircleOverlap(const glm::vec2 &aabb_pos, const glm::vec2 &aabb_size,
                                const glm::vec2 &circle_center, float circle_radius)
    {
        // 计算圆心到AABB的最邻近点
        glm::vec2 nearest_point = glm::clamp(circle_center, aabb_pos, aabb_pos + aabb_size);

        // 使用距离平方比较，避免开方运算
        float distance_squared = glm::length2(nearest_point - circle_center);
        return distance_squared < (circle_radius * circle_radius);
    }

    bool checkAABBOverlap(const glm::vec2 &a_pos, const glm::vec2 &a_size, const glm::vec2 &b_pos,
                          const glm::vec2 &b_size)
    {
        return a_pos.x < b_pos.x + b_size.x && a_pos.x + a_size.x > b_pos.x &&
                 a_pos.y < b_pos.y + b_size.y && a_pos.y + a_size.y > b_pos.y;
    }

    bool checkRectOverlap(const engine::utils::Rect &a, const engine::utils::Rect &b)
    {
        return checkAABBOverlap(a.position, a.size, b.position, b.size);
    }

    bool checkPointInCircle(const glm::vec2 &point, const glm::vec2 &center, float radius)
    {
        // 使用距离平方比较，避免开方运算
        float distance_squared = glm::length2(point - center);
        return distance_squared < (radius * radius);
    }

} // namespace engine::physics::collision
