#pragma once
#include "../utils/math.h"
#include <glm/vec2.hpp>
#include <optional>
#include <set>     // 用于 std::set
#include <unordered_map>
#include <utility> // 用于 std::pair
#include <vector>

namespace engine::component {
    class ColliderComponent;
    class PhysicsComponent;
    class TileLayerComponent;
    class TransformComponent;
    enum class TileType;
} // namespace engine::component

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace engine::physics {

    /**
     * @brief 负责管理和模拟物理行为及碰撞检测。
     */
    class PhysicsEngine {
    private:
        /**
         * @brief 空间网格结构体
         */
        struct SpatialGrid {
            float cell_size;
            float inv_cell_size; // 预计算倒数，避免重复除法
            std::unordered_map<int64_t,
                               std::vector<std::pair<engine::object::GameObject*,
                                                     engine::component::ColliderComponent*>>>
                grid;

            SpatialGrid(float size = 100.0f) : cell_size(size), inv_cell_size(1.0f / size)
            {
                grid.reserve(128); // 预分配空间
            }

            int64_t getGridKey(float x, float y) const;
            void clear();
            void insert(engine::object::GameObject* obj, engine::component::ColliderComponent* cc);
            // 获取指定区域内的所有网格键
            std::vector<int64_t> getNearbyKeys(float x, float y, float width, float height) const;
            // 获取对象覆盖的所有网格键
            std::vector<int64_t> getObjectGridKeys(float x, float y, float width,
                                                   float height) const;
        };

        /**
         * @brief 瓦片图层检测上下文结构体
         */
        struct TileCollisionContext {
            glm::vec2 displacement{0.0f};
            glm::vec2 new_position{0.0f};
            glm::vec2 world_aabb_position{0.0f};
            glm::vec2 world_aabb_size{0.0f};
            bool has_x_collision = false;
            bool has_y_collision = false;
        };

        /**
         * @brief 固体对象碰撞检测上下文结构体
         */
        struct SolidObjectCollisionContext {
            glm::vec2 move_center{0.0f};
            glm::vec2 solid_center{0.0f};
            glm::vec2 overlap{0.0f};
            glm::vec2 move_aabb_position{0.0f};
            glm::vec2 move_aabb_size{0.0f};
            glm::vec2 solid_aabb_position{0.0f};
            glm::vec2 solid_aabb_size{0.0f};
            bool has_collision = false;
        };

    private:
        std::vector<engine::component::PhysicsComponent*>
            components_;               ///< @brief 注册的物理组件容器，非拥有指针
        std::vector<engine::component::TileLayerComponent*>
            collision_tile_layers_;    ///< @brief 碰撞检测的瓦片图层容器
        glm::vec2 gravity_ = {0.0f,
                              980.0f}; ///< @brief 默认重力值 (像素/秒^2, 相当于100像素对应现实1m)
        float max_speed_ = 500.0f;     ///< @brief 最大速度
        std::optional<engine::utils::Rect> world_bounds_; ///< @brief 世界边界，用于限制物体运动

        std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>>
            collision_pairs_;      ///< @brief 物体碰撞对（每次 update 都会清空）
        std::vector<std::pair<engine::object::GameObject*, engine::component::TileType>>
            tile_trigger_events_;  ///< @brief 瓦片图层触发事件（每次 update 都会清空）
        SpatialGrid spatial_grid_; ///< @brief 空间网格

    public:
        PhysicsEngine() = default;

        // 禁止拷贝和移动语义
        PhysicsEngine(const PhysicsEngine &) = delete;
        PhysicsEngine &operator=(const PhysicsEngine &) = delete;
        PhysicsEngine(PhysicsEngine &&) = delete;
        PhysicsEngine &operator=(PhysicsEngine &&) = delete;

        void registerComponent(engine::component::PhysicsComponent* component);
        void unregisterComponent(engine::component::PhysicsComponent* component);

        void registerCollisionTileLayer(engine::component::TileLayerComponent* tile_layer);
        void unregisterCollisionTileLayer(engine::component::TileLayerComponent* tile_layer);

        void update(float delta_time);

        // --- getters ---
        const glm::vec2 &getGravity() const { return gravity_; }
        float getMaxSpeed() const { return max_speed_; }
        const std::optional<engine::utils::Rect> &getWorldBounds() const { return world_bounds_; }
        const std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>> &
        getCollisionPairs() const
        {
            return collision_pairs_;
        }
        const std::vector<std::pair<engine::object::GameObject*, engine::component::TileType>> &
        getTileTriggerEvents() const
        {
            return tile_trigger_events_;
        }

        // --- setters ---
        void setGravity(const glm::vec2 &gravity) { gravity_ = gravity; }
        void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
        void setWorldBounds(const engine::utils::Rect &bounds) { world_bounds_ = bounds; }

    private:
        /**
         * @brief 检查物体之间的碰撞
         */
        void checkObjectCollisions();

        /**
         * @brief 检查瓦片触发事件
         */
        void checkTileTriggers();

        /**
         * @brief 检查指定空间网格中的物体之间的碰撞
         */
        void checkCollisionsInCell(
            const std::vector<std::pair<engine::object::GameObject*,
                                        engine::component::ColliderComponent*>> &objects,
            std::set<std::pair<engine::object::GameObject*, engine::object::GameObject*>>
                &checked_pairs);

        /**
         * @brief 解决瓦片图层对象之间的碰撞
         */
        void resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time);

        /**
         * @brief 解决固体对象之间的碰撞
         */
        void resolveSolidObjectCollisions(engine::object::GameObject* move_obj,
                                          engine::object::GameObject* solid_obj);

        /**
         * @brief 应用世界边界限制
         */
        void applyWorldBounds(engine::component::PhysicsComponent* pc);

        // ==================== 处理瓦片碰撞函数声明 ====================

        /**
         * @brief 确认瓦片图层输入参数是否有效
         */
        bool validateTileCollisionInputs(engine::component::PhysicsComponent* pc,
                                         engine::component::TransformComponent*&tc,
                                         engine::component::ColliderComponent*&cc,
                                         TileCollisionContext &context) const;

        /**
         * @brief 计算瓦片图层对象在指定时间间隔内的位移
         */
        bool calculateTileDisplacement(engine::component::PhysicsComponent* pc,
                                       engine::component::TransformComponent*&tc,
                                       engine::component::ColliderComponent*&cc, float delta_time,
                                       TileCollisionContext &context) const;

        /**
         * @brief 解决瓦片图层对象在 X 轴上的碰撞
         */
        void resolveXAxisTileCollision(const engine::component::TileLayerComponent* layer,
                                       engine::component::PhysicsComponent* pc,
                                       TileCollisionContext &context) const;

        /**
         * @brief 解决瓦片图层对象在 Y 轴上的碰撞
         */
        void resolveYAxisTileCollision(const engine::component::TileLayerComponent* layer,
                                       engine::component::PhysicsComponent* pc,
                                       TileCollisionContext &context) const;

        /**
         * @brief 应用瓦片图层对象在指定时间间隔内的位移结果
         */
        void applyTileCollisionResults(engine::component::TransformComponent* tc,
                                       engine::component::PhysicsComponent* pc,
                                       const TileCollisionContext &context) const;

        // ==================== 辅助函数声明 ====================

        /**
         * @brief 检查瓦片类型是否为斜坡瓦片
         */
        bool isSlopeTile(engine::component::TileType tile_type) const;

        /**
         * @brief 检查瓦片类型是否为固体瓦片
         */
        bool isSolidTile(engine::component::TileType tile_type) const;

        /**
         * @brief 检查瓦片类型是否为地面瓦片（固体或单向平台）
         */
        bool isGroundTile(engine::component::TileType tile_type) const;

        /**
         * @brief 检查瓦片类型是否为梯子瓦片
         */
        bool isLadderTile(engine::component::TileType tile_type) const;

        /**
         * @brief 根据瓦片类型和指定宽度的 x 坐标，计算瓦片上对应的 y 坐标
         */
        float getTileHeightAtWidth(float width, engine::component::TileType tile_type,
                                   glm::vec2 tile_size) const;

        /**
         * @brief 处理X轴固体瓦片碰撞
         */
        void handleSolidCollisionX(bool moving_right, int tile_x, const glm::vec2 &tile_size,
                                   engine::component::PhysicsComponent* pc,
                                   TileCollisionContext &context) const;

        /**
         * @brief 处理X轴斜坡瓦片碰撞
         */
        void handleSlopeCollisionX(bool moving_right, int tile_x, int tile_y_bottom,
                                   engine::component::TileType tile_type_bottom,
                                   const glm::vec2 &tile_size,
                                   engine::component::PhysicsComponent* pc,
                                   TileCollisionContext &context) const;

        /**
         * @brief 处理Y轴地面碰撞
         */
        void handleGroundCollisionY(int tile_y, const glm::vec2 &tile_size,
                                    engine::component::PhysicsComponent* pc,
                                    TileCollisionContext &context) const;

        /**
         * @brief 处理Y轴梯子碰撞
         */
        void handleLadderCollisionY(int tile_y, const glm::vec2 &tile_size,
                                    engine::component::PhysicsComponent* pc,
                                    TileCollisionContext &context) const;

        /**
         * @brief 处理Y轴天花板碰撞
         */
        void handleCeilingCollisionY(int tile_y, const glm::vec2 &tile_size,
                                     engine::component::PhysicsComponent* pc,
                                     TileCollisionContext &context) const;

        /**
         * @brief 处理Y轴斜坡瓦片碰撞
         */
        void handleSlopeCollisionY(int tile_x_left, int tile_x_right, int tile_y,
                                   engine::component::TileType tile_type_left,
                                   engine::component::TileType tile_type_right,
                                   const glm::vec2 &tile_size,
                                   engine::component::PhysicsComponent* pc,
                                   TileCollisionContext &context) const;

        // ==================== 处理固体对象碰撞函数声明 ====================

        /**
         * @brief 验证固体对象碰撞输入参数
         */
        bool validateSolidObjectCollisionInputs(engine::object::GameObject* move_obj,
                                                engine::object::GameObject* solid_obj,
                                                engine::component::TransformComponent*&move_tc,
                                                engine::component::PhysicsComponent*&move_pc,
                                                engine::component::ColliderComponent*&move_cc,
                                                engine::component::ColliderComponent*&solid_cc,
                                                SolidObjectCollisionContext &context) const;

        /**
         * @brief 计算固体对象碰撞数据
         */
        bool calculateSolidObjectCollisionData(SolidObjectCollisionContext &context) const;

        /**
         * @brief 应用固体对象碰撞结果
         */
        void applySolidObjectCollisionResults(engine::component::TransformComponent* move_tc,
                                              engine::component::PhysicsComponent* move_pc,
                                              const SolidObjectCollisionContext &context) const;
    };
} // namespace engine::physics
