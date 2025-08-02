#include "game_scene.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/render/camera.h"
#include "../../engine/scene/level_loader.h"
#include <SDL3/SDL_rect.h>
#include <spdlog/spdlog.h>

namespace game::scenes {
    GameScene::GameScene(const std::string_view &name, engine::core::Context &context,
                         engine::scene::SceneManager &scene_manager)
        : Scene(name, context, scene_manager)
    {
        spdlog::trace("GameScene 构造完成");
    }

    void GameScene::init()
    {
        engine::scene::LevelLoader loader;
        loader.loadLevel("assets/maps/level1.tmj", *this);

        // 创建 test_object
        createTestObject();

        Scene::init();
        spdlog::trace("GameScene 初始化完成。");
    }

    void GameScene::update(double delta_time)
    {
        Scene::update(delta_time);
    }

    void GameScene::render()
    {
        Scene::render();
    }

    void GameScene::handleInput()
    {
        Scene::handleInput();
        testCamera();
    }

    void GameScene::clean()
    {
        Scene::clean();
    }

    void GameScene::createTestObject()
    {
        spdlog::trace("在 GameScene 中创建 test_object...");
        auto test_object = std::make_unique<engine::object::GameObject>("test_object");

        // 添加组件
        test_object->addComponent<engine::component::TransformComponent>(glm::vec2(100.0f, 100.0f));
        test_object->addComponent<engine::component::SpriteComponent>(
            "assets/textures/Props/big-crate.png", context_.getResourceManager());

        // 将创建好的 GameObject 添加到场景中
        addGameObject(std::move(test_object));
        spdlog::trace("test_object 创建并添加到 GameScene 中。");
    }

    void GameScene::testCamera()
    {
        auto &camera = context_.getCamera();
        auto &input_manager = context_.getInputManager();
        if (input_manager.isActionHeldDown("move_up")) camera.move(glm::vec2(0, -1));
        if (input_manager.isActionHeldDown("move_down")) camera.move(glm::vec2(0, 1));
        if (input_manager.isActionHeldDown("move_left")) camera.move(glm::vec2(-1, 0));
        if (input_manager.isActionHeldDown("move_right")) camera.move(glm::vec2(1, 0));
    }

} // namespace game::scenes
