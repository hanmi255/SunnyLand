#pragma once
#include "../../engine/scene/scene.h"
#include <glm/vec2.hpp>
#include <memory>
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

        void handleObjectCollisions();
        void handleTileTriggers();
        void PlayerVSEnemyCollision(engine::object::GameObject* player,
                                    engine::object::GameObject* enemy);
        void PlayerVSItemCollision(engine::object::GameObject* player,
                                   engine::object::GameObject* item);

        void createEffect(const glm::vec2 &center_pos, const std::string_view &tag);
    };

} // namespace game::scene
