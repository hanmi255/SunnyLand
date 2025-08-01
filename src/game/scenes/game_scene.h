#pragma once
#include "../../engine/scene/scene.h"
#include <memory>

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace game::scenes {

    /**
     * @brief 主要的游戏场景，包含玩家、敌人、关卡元素等。
     */
    class GameScene final : public engine::scene::Scene {
    public:
        GameScene(const std::string_view &name, engine::core::Context &context,
                  engine::scene::SceneManager &scene_manager);

        void init() override;
        void update(double delta_time) override;
        void render() override;
        void handleInput() override;
        void clean() override;

    private:
        // 测试函数
        void createTestObject();
    };

} // namespace game::scenes
