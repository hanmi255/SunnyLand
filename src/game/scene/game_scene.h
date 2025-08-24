#pragma once
#include "../../engine/scene/scene.h"
#include <glm/vec2.hpp>
#include <memory>
#include <string_view>

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace engine::ui {
    class UILabel;
    class UIPanel;
} // namespace engine::ui

namespace game::data {
    class SessionData;
} // namespace game::data

namespace game::scene {

    /**
     * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
     */
    class GameScene final : public engine::scene::Scene {
        std::shared_ptr<game::data::SessionData>
            game_session_data_;                        ///< @brief 场景间共享数据，因此用shared_ptr
        engine::object::GameObject* player_ = nullptr; ///< @brief 玩家
        engine::ui::UILabel* score_label_ = nullptr;   ///< @brief 得分标签
        engine::ui::UIPanel* health_panel_ = nullptr;  ///< @brief 生命值图标面板

    private:
        struct EnemyConfig {
            static constexpr float EAGLE_FLIGHT_RANGE = 80.0F;
            static constexpr float FROG_JUMP_RANGE = 90.0F;
            static constexpr float FROG_OFFSET = 10.0F;
            static constexpr float OPOSSUM_PATROL_RANGE = 200.0F;
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
        GameScene(engine::core::Context &context, engine::scene::SceneManager &scene_manager,
                  std::shared_ptr<game::data::SessionData> data = nullptr);

        void init() override;
        void update(float delta_time) override;
        void render() override;
        void handleInput() override;
        void clean() override;

    private:
        [[nodiscard]] bool initLevel();
        [[nodiscard]] bool initPlayer();
        [[nodiscard]] bool initEnemyAndItem();
        [[nodiscard]] bool initUI();
        void initAudio();

        void handleObjectCollisions();
        void handleTileTriggers();
        void checkPlayerFallOutOfWorld();
        void handlePlayerDamage(int damage_amount);
        void PlayerVSEnemyCollision(engine::object::GameObject* player,
                                    engine::object::GameObject* enemy);
        void PlayerVSItemCollision(engine::object::GameObject* player,
                                   engine::object::GameObject* item);

        void toNextLevel(engine::object::GameObject* trigger);
        void showEndScene(bool is_win);

        [[nodiscard]] static std::string levelNameToPath(std::string_view level_name)
        {
            return "assets/maps/" + std::string(level_name) + ".tmj";
        }

        void createEffect(glm::vec2 center_pos, std::string_view tag);

        // --- UI 相关函数 ---
        void createScoreUI();
        void createHealthUI();
        void addScoreWithUI(int score);
        void healWithUI(int amount);
        void updateHealthWithUI();
    };

} // namespace game::scene
