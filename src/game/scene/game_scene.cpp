#include "game_scene.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/render/camera.h"
#include "../../engine/scene/level_loader.h"
#include "../component/player_component.h"
#include <SDL3/SDL_rect.h>
#include <spdlog/spdlog.h>
#include <string_view>
#include <unordered_map>

namespace game::scene {
    GameScene::GameScene(const std::string_view &name, engine::core::Context &context,
                         engine::scene::SceneManager &scene_manager)
        : Scene(name, context, scene_manager)
    {
        spdlog::trace("GameScene 构造完成");
    }

    void GameScene::init()
    {
        if (is_initialized_) {
            spdlog::warn("GameScene 已经初始化完成，重复调用 init()。");
            return;
        }
        spdlog::trace("GameScene 初始化开始...");

        if (!initLevel()) {
            spdlog::error("关卡初始化失败，无法继续。");
            context_.getInputManager().setShouldQuit(true);
            return;
        }
        if (!initPlayer()) {
            spdlog::error("玩家初始化失败，无法继续。");
            context_.getInputManager().setShouldQuit(true);
            return;
        }
        if (!initEnemyAndItem()) {
            spdlog::error("敌人和道具初始化失败，无法继续。");
            context_.getInputManager().setShouldQuit(true);
            return;
        }

        Scene::init();
        spdlog::trace("GameScene 初始化完成。");
    }

    void GameScene::update(float delta_time)
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
    }

    void GameScene::clean()
    {
        Scene::clean();
    }

    bool GameScene::initLevel()
    {
        // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
        engine::scene::LevelLoader level_loader;
        if (!level_loader.loadLevel("assets/maps/level1.tmj", *this)) {
            spdlog::error("关卡加载失败");
            return false;
        }

        // 注册"main"层到物理引擎
        auto* main_layer = findGameObjectByName("main");
        if (!main_layer) {
            spdlog::error("未找到\"main\"层");
            return false;
        }
        auto* tile_layer = main_layer->getComponent<engine::component::TileLayerComponent>();
        if (!tile_layer) {
            spdlog::error("\"main\"层没有 TileLayerComponent 组件");
            return false;
        }
        context_.getPhysicsEngine().registerCollisionTileLayer(tile_layer);
        spdlog::info("注册\"main\"层到物理引擎");

        // 设置相机边界
        auto world_size =
            main_layer->getComponent<engine::component::TileLayerComponent>()->getWorldSize();
        context_.getCamera().setLimitBounds(engine::utils::Rect(glm::vec2(0.0f), world_size));

        // 设置世界边界
        context_.getPhysicsEngine().setWorldBounds(
            engine::utils::Rect(glm::vec2(0.0f), world_size));

        spdlog::trace("关卡初始化完成。");
        return true;
    }

    bool GameScene::initPlayer()
    {
        // 获取玩家对象
        player_ = findGameObjectByName("player");
        if (!player_) {
            spdlog::error("未找到玩家对象");
            return false;
        }

        // 添加PlayerComponent到玩家对象
        auto* player_component = player_->addComponent<game::component::PlayerComponent>();
        if (!player_component) {
            spdlog::error("无法添加 PlayerComponent 到玩家对象");
            return false;
        }

        // 相机跟随玩家
        auto* player_transform = player_->getComponent<engine::component::TransformComponent>();
        if (!player_transform) {
            spdlog::error("玩家对象没有 TransformComponent 组件, 无法设置相机目标");
            return false;
        }
        context_.getCamera().setTarget(player_transform);
        spdlog::trace("Player初始化完成。");
        return true;
    }

    bool GameScene::initEnemyAndItem()
    {
        // 定义敌人类型到动画名称的映射
        static const std::unordered_map<std::string_view, std::string_view> enemy_animations = {
            {"eagle", "fly"},
            {"frog", "idle"},
            {"opossum", "walk"}
        };

        bool success = true;
        for (auto &game_object : game_objects_) {
            // 检查是否为敌人类型
            const auto enemy_it = enemy_animations.find(game_object->getName());
            if (enemy_it != enemy_animations.end()) {
                if (auto ac = game_object->getComponent<engine::component::AnimationComponent>();
                    ac) {
                    ac->playAnimation(std::string(enemy_it->second));
                } else {
                    spdlog::error("{}对象缺少 AnimationComponent，无法播放动画。",
                                  game_object->getName());
                    success = false;
                }
            }
            // 检查是否为道具
            else if (game_object->getTag() == "item") {
                if (auto ac = game_object->getComponent<engine::component::AnimationComponent>();
                    ac) {
                    ac->playAnimation("idle");
                } else {
                    spdlog::error("Item对象缺少 AnimationComponent，无法播放动画。");
                    success = false;
                }
            }
        }
        return success;
    }

} // namespace game::scene
