#pragma once
#include "../../engine/scene/scene.h"
#include <glm/vec2.hpp>
#include <string_view>

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace game::scene {

    /**
     * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
     */
    class GameScene final : public engine::scene::Scene {
        engine::object::GameObject* player_ = nullptr;

    private:
        struct EnemyConfig {
            static constexpr float EAGLE_FLIGHT_RANGE = 80.0f;
            static constexpr float FROG_JUMP_RANGE = 90.0f;
            static constexpr float FROG_OFFSET = 10.0f;
            static constexpr float OPOSSUM_PATROL_RANGE = 200.0f;
        };

        struct EffectConfig {
            std::string_view texture_path;
            int frame_count;
            float frame_width;
            float frame_height;
            float frame_duration;

            constexpr EffectConfig(std::string_view path, int count, float w, float h,
                                   float duration)
                : texture_path(path)
                , frame_count(count)
                , frame_width(w)
                , frame_height(h)
                , frame_duration(duration)
            {
            }

            // 预定义的特效配置
            static const EffectConfig ENEMY;
            static const EffectConfig ITEM;
        };

    public:
        GameScene(const std::string_view &name, engine::core::Context &context,
                  engine::scene::SceneManager &scene_manager);

        void init() override;
        void update(float delta_time) override;
        void render() override;
        void handleInput() override;
        void clean() override;

    private:
        [[nodiscard]] bool initLevel();
        [[nodiscard]] bool initPlayer();
        [[nodiscard]] bool initEnemyAndItem();
        void initAudio();

        void handleObjectCollisions();
        void handleTileTriggers();
        void PlayerVSEnemyCollision(engine::object::GameObject* player,
                                    engine::object::GameObject* enemy);
        void PlayerVSItemCollision(engine::object::GameObject* player,
                                   engine::object::GameObject* item);

        void createEffect(const glm::vec2 &center_pos, const std::string_view &tag);
    };

} // namespace game::scene
