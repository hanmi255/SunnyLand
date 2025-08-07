#include "game_scene.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/render/animation.h"
#include "../../engine/render/camera.h"
#include "../../engine/scene/level_loader.h"
#include "../component/player_component.h"
#include <SDL3/SDL_rect.h>
#include <spdlog/spdlog.h>
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
        handleObjectCollisions();
        handleTileTriggers();
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
            {"eagle", "fly"}, {"frog", "idle"}, {"opossum", "walk"}};

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

    void GameScene::handleObjectCollisions()
    {
        // 从物理引擎中获取碰撞对
        const auto &collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
        for (const auto &[obj1, obj2] : collision_pairs) {
            // 确定玩家对象和另一个对象
            const auto [player, other] = [obj1, obj2]() {
                if (obj1->getName() == "player") return std::make_pair(obj1, obj2);
                if (obj2->getName() == "player") return std::make_pair(obj2, obj1);
                return std::make_pair<engine::object::GameObject*, engine::object::GameObject*>(
                    nullptr, nullptr);
            }();

            // 如果没有玩家参与碰撞，跳过
            if (!player) continue;

            const auto &other_tag = other->getTag();
            [this, player, other, &other_tag]() {
                if (other_tag == "enemy") {
                    PlayerVSEnemyCollision(player, other);
                } else if (other_tag == "item") {
                    PlayerVSItemCollision(player, other);
                } else if (other_tag == "hazard") {
                    if (auto* player_component =
                            player->getComponent<game::component::PlayerComponent>()) {
                        player_component->takeDamage(1);
                        spdlog::debug("玩家 {} 受到了 HAZARD 对象伤害", player->getName());
                    }
                }
            }();
        }
    }

    void GameScene::handleTileTriggers()
    {
        const auto &tile_trigger_events = context_.getPhysicsEngine().getTileTriggerEvents();
        for (const auto &event : tile_trigger_events) {
            auto obj = event.first;
            auto tile_type = event.second;
            if (tile_type == engine::component::TileType::HAZARD) {
                // 玩家碰到到危险瓦片，受伤
                if (obj->getName() == "player") {
                    obj->getComponent<game::component::PlayerComponent>()->takeDamage(1);
                    spdlog::debug("玩家 {} 受到了 HAZARD 瓦片伤害", obj->getName());
                }
                // TODO: 其他对象类型的处理，目前让敌人无视瓦片伤害
            }
        }
    }

    void GameScene::PlayerVSEnemyCollision(engine::object::GameObject* player,
                                           engine::object::GameObject* enemy)
    {
        // --- 踩踏判断逻辑：1. 玩家中心点在敌人上方    2. 重叠区域：overlap.x > overlap.y ---
        auto player_aabb =
            player->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
        auto enemy_aabb =
            enemy->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
        auto player_center = player_aabb.position + player_aabb.size / 2.0f;
        auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;
        auto overlap = glm::vec2(player_aabb.size / 2.0f + enemy_aabb.size / 2.0f) -
                       glm::abs(player_center - enemy_center);

        // 踩踏判断成功，敌人受伤
        if (overlap.x > overlap.y && player_center.y < enemy_center.y) {
            spdlog::info("玩家 {} 踩踏了敌人 {}", player->getName(), enemy->getName());
            auto enemy_health = enemy->getComponent<engine::component::HealthComponent>();
            if (!enemy_health) {
                spdlog::error("敌人 {} 没有 HealthComponent 组件，无法处理踩踏伤害",
                              enemy->getName());
                return;
            }
            enemy_health->takeDamage(1); // 造成1点伤害
            if (!enemy_health->isAlive()) {
                spdlog::info("敌人 {} 被踩踏后死亡", enemy->getName());
                enemy->setNeedRemove(true);                  // 标记敌人为待删除状态
                createEffect(enemy_center, enemy->getTag()); // 创建（死亡）特效
            }
            // 玩家跳起效果
            player->getComponent<engine::component::PhysicsComponent>()->velocity_.y =
                -300.0f; // 向上跳起
        }
        // 踩踏判断失败，玩家受伤
        else {
            spdlog::info("敌人 {} 对玩家 {} 造成伤害", enemy->getName(), player->getName());
            player->getComponent<game::component::PlayerComponent>()->takeDamage(1);
            // TODO: 其他受伤逻辑
        }
    }

    void GameScene::PlayerVSItemCollision(engine::object::GameObject* player,
                                          engine::object::GameObject* item)
    {
        if (item->getName() == "fruit") {
            player->getComponent<engine::component::HealthComponent>()->heal(1); // 加血
        } else if (item->getName() == "gem") {
            // TODO: 加分
        }
        item->setNeedRemove(true); // 标记道具为待删除状态
        auto item_aabb = item->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
        createEffect(item_aabb.position + item_aabb.size / 2.0f, item->getTag());
    }

    void GameScene::createEffect(const glm::vec2 &center_pos, const std::string_view &tag)
    {
        // --- 创建游戏对象和变换组件 ---
        auto effect_obj =
            std::make_unique<engine::object::GameObject>("effect_" + std::string(tag));
        effect_obj->addComponent<engine::component::TransformComponent>(center_pos);

        // --- 根据标签创建不同的精灵组件和动画---
        auto animation = std::make_unique<engine::render::Animation>("effect", false);
        if (tag == "enemy") {
            effect_obj->addComponent<engine::component::SpriteComponent>(
                "assets/textures/FX/enemy-deadth.png", context_.getResourceManager(),
                engine::utils::Alignment::CENTER);
            for (auto i = 0; i < 5; ++i) {
                animation->addFrame({static_cast<float>(i * 40), 0.0f, 40.0f, 41.0f}, 0.1f);
            }
        } else if (tag == "item") {
            effect_obj->addComponent<engine::component::SpriteComponent>(
                "assets/textures/FX/item-feedback.png", context_.getResourceManager(),
                engine::utils::Alignment::CENTER);
            for (auto i = 0; i < 4; ++i) {
                animation->addFrame({static_cast<float>(i * 32), 0.0f, 32.0f, 32.0f}, 0.1f);
            }
        } else {
            spdlog::warn("未知特效类型: {}", tag);
            return;
        }

        // --- 根据创建的动画，添加动画组件，并设置为单次播放 ---
        auto animation_component =
            effect_obj->addComponent<engine::component::AnimationComponent>();
        animation_component->addAnimation(std::move(animation));
        animation_component->setOneShotRemoval(true);
        animation_component->playAnimation("effect");
        safelyAddGameObject(std::move(effect_obj));
        spdlog::debug("创建特效: {}", tag);
    }

} // namespace game::scene
