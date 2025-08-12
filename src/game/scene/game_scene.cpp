#include "game_scene.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/render/animation.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/text_renderer.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_label.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/utils/math.h"
#include "../component/ai_behavior/jump_behavior.h"
#include "../component/ai_behavior/patrol_behavior.h"
#include "../component/ai_behavior/updown_behavior.h"
#include "../component/ai_component.h"
#include "../component/player_component.h"
#include "../data/session_data.h"
#include "end_scene.h"
#include "menu_scene.h"
#include <SDL3/SDL_rect.h>
#include <spdlog/spdlog.h>

namespace game::scene {
    // 定义EffectConfig的静态成员
    const GameScene::EffectConfig GameScene::EffectConfig::ENEMY{
        "assets/textures/FX/enemy-deadth.png", 5, 40.0f, 41.0f, 0.1f};
    const GameScene::EffectConfig GameScene::EffectConfig::ITEM{
        "assets/textures/FX/item-feedback.png", 4, 32.0f, 32.0f, 0.1f};

    GameScene::GameScene(engine::core::Context &context, engine::scene::SceneManager &scene_manager,
                         std::shared_ptr<game::data::SessionData> data)
        : Scene("GameScene", context, scene_manager), game_session_data_(std::move(data))
    {
        if (!game_session_data_) { // 如果没有传入SessionData，则创建一个默认的
            game_session_data_ = std::make_shared<game::data::SessionData>();
            spdlog::info("未提供 SessionData，使用默认值。");
        }
        spdlog::trace("GameScene 构造完成。");
    }
    void GameScene::init()
    {
        if (is_initialized_) {
            spdlog::warn("GameScene 已经初始化完成，重复调用 init()。");
            return;
        }

        spdlog::trace("GameScene 初始化开始...");
        context_.getGameState().setState(engine::core::State::Playing);
        game_session_data_->syncHighScore("assets/data/save.json");

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
        if (!initUI()) {
            spdlog::error("UI初始化失败，无法继续。");
            context_.getInputManager().setShouldQuit(true);
            return;
        }

        initAudio();

        Scene::init();
        spdlog::trace("GameScene 初始化完成。");
    }

    void GameScene::update(float delta_time)
    {
        Scene::update(delta_time);
        handleObjectCollisions();
        handleTileTriggers();
        checkPlayerFallOutOfWorld();
    }

    void GameScene::render()
    {
        Scene::render();
    }

    void GameScene::handleInput()
    {
        Scene::handleInput();
        // 检查暂停动作
        if (context_.getInputManager().isActionJustPressed("pause")) {
            spdlog::debug("在 GameScene 中检测到暂停动作，正在推送 MenuScene。");
            scene_manager_.requestPushScene(
                std::make_unique<MenuScene>(context_, scene_manager_, game_session_data_));
        }
    }

    void GameScene::clean()
    {
        Scene::clean();
    }

    bool GameScene::initLevel()
    {
        // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
        engine::scene::LevelLoader level_loader;
        auto level_path = game_session_data_->getMapPath();
        if (!level_loader.loadLevel(level_path, *this)) {
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
        context_.getCamera().setPosition(glm::vec2(0.0f)); // 开始时重置相机位置，以免切换场景时晃动

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

        // 从SessionData中更新玩家生命值
        auto* health_component = player_->getComponent<engine::component::HealthComponent>();
        if (!health_component) {
            spdlog::error("玩家对象缺少 HealthComponent 组件，无法设置生命值");
            return false;
        }
        health_component->setMaxHealth(game_session_data_->getMaxHealth());
        health_component->setCurrentHealth(game_session_data_->getCurrentHealth());

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
        bool success = true;

        for (const auto &obj : game_objects_) {
            const auto &name = obj->getName();

            // 初始化敌人AI
            if (name == "eagle" || name == "frog" || name == "opossum") {
                auto* ai = obj->addComponent<game::component::AIComponent>();
                auto* transform = obj->getComponent<engine::component::TransformComponent>();

                if (!ai || !transform) {
                    spdlog::error("{}缺少必要组件", name);
                    success = false;
                    continue;
                }

                const auto pos = transform->getPosition();

                if (name == "eagle") {
                    float y_max = pos.y;
                    float y_min = y_max - EnemyConfig::EAGLE_FLIGHT_RANGE;
                    ai->setBehavior(std::make_unique<game::component::ai_behavior::UpDownBehavior>(
                        y_min, y_max));

                } else if (name == "frog") {
                    float x_max = pos.x - EnemyConfig::FROG_OFFSET;
                    float x_min = x_max - EnemyConfig::FROG_JUMP_RANGE;
                    ai->setBehavior(
                        std::make_unique<game::component::ai_behavior::JumpBehavior>(x_min, x_max));

                } else if (name == "opossum") {
                    float x_max = pos.x;
                    float x_min = x_max - EnemyConfig::OPOSSUM_PATROL_RANGE;
                    ai->setBehavior(std::make_unique<game::component::ai_behavior::PatrolBehavior>(
                        x_min, x_max));
                }
            }

            // 初始化道具动画
            if (obj->getTag() == "item") {
                auto* ac = obj->getComponent<engine::component::AnimationComponent>();
                if (ac) {
                    ac->playAnimation("idle");
                } else {
                    spdlog::error("Item对象'{}'缺少AnimationComponent", obj->getName());
                    success = false;
                }
            }
        }

        return success;
    }

    bool GameScene::initUI()
    {
        if (!ui_manager_->init(context_.getGameState().getLogicalSize())) return false;

        createScoreUI();
        createHealthUI();

        return true;
    }

    void GameScene::initAudio()
    {
        // 播放背景音乐 (循环，淡入1秒)
        context_.getAudioPlayer().playMusic("assets/audio/hurry_up_and_run.ogg", true, 1000);
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
                    handlePlayerDamage(1);
                }
                // 处理玩家与关底触发器碰撞
                else if (other_tag == "next_level") {
                    toNextLevel(other);
                }
                // 处理玩家与胜利触发器碰撞
                else if (other_tag == "win") {
                    showEndScene(true);
                }
            }();
        }
    }

    void GameScene::handleTileTriggers()
    {
        const auto &tile_trigger_events = context_.getPhysicsEngine().getTileTriggerEvents();
        for (const auto &[obj, tile_type] : tile_trigger_events) {
            if (tile_type == engine::component::TileType::HAZARD) {
                // 玩家碰到到危险瓦片，受伤
                if (obj->getName() == "player") {
                    handlePlayerDamage(1);
                    spdlog::debug("玩家 {} 受到了 HAZARD 瓦片伤害", obj->getName());
                }
            }
            // TODO: 其他对象类型的处理，目前让敌人无视瓦片伤害
        }
    }

    void GameScene::checkPlayerFallOutOfWorld()
    {
        // 玩家掉出地图下方则判断为失败
        if (!player_) {
            return;
        }

        auto pos = player_->getComponent<engine::component::TransformComponent>()->getPosition();
        auto world_rect = context_.getPhysicsEngine().getWorldBounds();
        // 多100像素冗余量
        if (world_rect && pos.y > world_rect->position.y + world_rect->size.y + 100.0f) {
            spdlog::debug("玩家掉出地图下方，游戏失败");
            showEndScene(false);
        }
    }

    void GameScene::handlePlayerDamage(int damage_amount)
    {
        auto* player_component = player_->getComponent<game::component::PlayerComponent>();
        if (!player_component->takeDamage(damage_amount)) { // 没有受伤，直接返回
            return;
        }
        if (player_component->isDead()) {
            spdlog::info("玩家 {} 死亡", player_->getName());
            // TODO: 可能的死亡逻辑处理
        }
        // 更新游戏数据和 UI
        updateHealthWithUI();
    }

    void GameScene::PlayerVSEnemyCollision(engine::object::GameObject* player,
                                           engine::object::GameObject* enemy)
    {
        // 空指针检查
        if (!player || !enemy) {
            spdlog::error("PlayerVSEnemyCollision: player或enemy为nullptr");
            return;
        }

        // 获取碰撞组件
        auto* player_collider = player->getComponent<engine::component::ColliderComponent>();
        auto* enemy_collider = enemy->getComponent<engine::component::ColliderComponent>();

        if (!player_collider) {
            spdlog::error("玩家 {} 没有 ColliderComponent 组件", player->getName());
            return;
        }

        if (!enemy_collider) {
            spdlog::error("敌人 {} 没有 ColliderComponent 组件", enemy->getName());
            return;
        }

        // --- 踩踏判断逻辑：1. 玩家中心点在敌人上方    2. 重叠区域：overlap.x > overlap.y ---
        const auto player_aabb = player_collider->getWorldAABB();
        const auto enemy_aabb = enemy_collider->getWorldAABB();

        const auto player_center = player_aabb.position + player_aabb.size / 2.0f;
        const auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;

        const auto overlap = glm::vec2(player_aabb.size / 2.0f + enemy_aabb.size / 2.0f) -
                             glm::abs(player_center - enemy_center);

        // 踩踏判断成功，敌人受伤
        if (overlap.x > overlap.y && player_center.y < enemy_center.y) {
            spdlog::info("玩家 {} 踩踏了敌人 {}", player->getName(), enemy->getName());

            auto* enemy_health = enemy->getComponent<engine::component::HealthComponent>();
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
            auto* player_physics = player->getComponent<engine::component::PhysicsComponent>();
            if (player_physics) {
                player_physics->velocity_.y = -300.0f; // 向上跳起
                context_.getAudioPlayer().playSound("assets/audio/punch2a.mp3");
                // 加分
                addScoreWithUI(10);
            } else {
                spdlog::warn("玩家 {} 没有 PhysicsComponent 组件，无法执行跳起效果",
                             player->getName());
            }
        }
        // 踩踏判断失败，玩家受伤
        else {
            spdlog::info("敌人 {} 对玩家 {} 造成伤害", enemy->getName(), player->getName());
            handlePlayerDamage(1);
        }
    }

    void GameScene::PlayerVSItemCollision(engine::object::GameObject*,
                                          engine::object::GameObject* item)
    {
        if (item->getName() == "fruit") {
            healWithUI(1);         // 加血
        } else if (item->getName() == "gem") {
            addScoreWithUI(5);     // 加分
        }
        item->setNeedRemove(true); // 标记道具为待删除状态
        auto item_aabb = item->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
        createEffect(item_aabb.position + item_aabb.size / 2.0f, item->getTag());
        context_.getAudioPlayer().playSound("assets/audio/poka01.mp3");
    }

    void GameScene::toNextLevel(engine::object::GameObject* trigger)
    {
        auto scene_name = trigger->getName();
        auto map_path = levelNameToPath(scene_name);
        game_session_data_->setNextLevel(map_path);
        auto next_scene = std::make_unique<GameScene>(context_, scene_manager_, game_session_data_);
        scene_manager_.requestReplaceScene(std::move(next_scene));
    }

    void GameScene::showEndScene(bool is_win)
    {
        spdlog::debug("显示结束场景，游戏 {}", is_win ? "胜利" : "失败");
        game_session_data_->setIsWin(is_win);
        auto end_scene =
            std::make_unique<game::scene::EndScene>(context_, scene_manager_, game_session_data_);
        scene_manager_.requestPushScene(std::move(end_scene));
    }

    void GameScene::createEffect(const glm::vec2 &center_pos, const std::string_view &tag)
    {
        // 根据标签获取配置
        const EffectConfig* config = nullptr;
        if (tag == "enemy") {
            config = &EffectConfig::ENEMY;
        } else if (tag == "item") {
            config = &EffectConfig::ITEM;
        } else {
            spdlog::warn("未知特效类型: {}", tag);
            return;
        }

        // 创建游戏对象
        auto effect_obj =
            std::make_unique<engine::object::GameObject>("effect_" + std::string(tag));
        effect_obj->addComponent<engine::component::TransformComponent>(center_pos);

        // 添加精灵组件
        effect_obj->addComponent<engine::component::SpriteComponent>(
            std::string(config->texture_path), context_.getResourceManager(),
            engine::utils::Alignment::CENTER);

        // 创建动画
        auto animation = std::make_unique<engine::render::Animation>("effect", false);
        for (int i = 0; i < config->frame_count; ++i) {
            animation->addFrame({static_cast<float>(i) * config->frame_width, 0.0f,
                                 config->frame_width, config->frame_height},
                                config->frame_duration);
        }

        // --- 根据创建的动画，添加动画组件，并设置为单次播放 ---
        auto* animation_component =
            effect_obj->addComponent<engine::component::AnimationComponent>();
        animation_component->addAnimation(std::move(animation));
        animation_component->setOneShotRemoval(true);
        animation_component->playAnimation("effect");
        safelyAddGameObject(std::move(effect_obj));
        spdlog::debug("创建特效: {}", tag);
    }

    void GameScene::createScoreUI()
    {
        // 创建得分标签
        const auto score_text = "Score: " + std::to_string(game_session_data_->getCurrentScore());
        auto score_label = std::make_unique<engine::ui::UILabel>(
            context_.getTextRenderer(), score_text, "assets/fonts/VonwaonBitmap-16px.ttf", 16);

        // 检查score_label是否成功创建
        if (!score_label) {
            spdlog::error("无法创建得分标签");
            return;
        }

        score_label_ = score_label.get();

        // 获取屏幕尺寸并检查根元素是否存在
        auto* root_element = ui_manager_->getRootElement();
        if (!root_element) {
            spdlog::error("UI根元素不存在，无法设置得分标签位置");
            return;
        }

        const auto screen_size = root_element->getSize();
        score_label_->setPosition(glm::vec2(screen_size.x - 100.0f, 10.0f));
        ui_manager_->addElement(std::move(score_label));

        spdlog::trace("得分UI创建完成");
    }

    void GameScene::createHealthUI()
    {
        const int max_health = game_session_data_->getMaxHealth();
        const int current_health = game_session_data_->getCurrentHealth();
        constexpr float start_x = 10.0f;
        constexpr float start_y = 10.0f;
        constexpr float icon_width = 20.0f;
        constexpr float icon_height = 18.0f;
        constexpr float spacing = 5.0f;
        constexpr std::string_view full_heart_tex = "assets/textures/UI/Heart.png";
        constexpr std::string_view empty_heart_tex = "assets/textures/UI/Heart-bg.png";

        // 创建一个默认的UIPanel (不需要背景色，因此大小无所谓，只用于定位)
        auto health_panel = std::make_unique<engine::ui::UIPanel>();
        if (!health_panel) {
            spdlog::error("无法创建生命值UI面板");
            return;
        }
        health_panel_ = health_panel.get(); // 成员变量赋值（获取裸指针）

        // --- 根据最大生命值，循环创建生命值图标(添加到UIPanel中) ---
        for (int i = 0; i < max_health; ++i) { // 创建背景图标
            const glm::vec2 icon_pos = {start_x + i * (icon_width + spacing), start_y};
            const glm::vec2 icon_size = {icon_width, icon_height};

            auto bg_icon =
                std::make_unique<engine::ui::UIImage>(empty_heart_tex, icon_pos, icon_size);

            health_panel_->addChild(std::move(bg_icon));
        }

        for (int i = 0; i < max_health; ++i) { // 创建前景图标
            const glm::vec2 icon_pos = {start_x + i * (icon_width + spacing), start_y};
            const glm::vec2 icon_size = {icon_width, icon_height};

            auto fg_icon =
                std::make_unique<engine::ui::UIImage>(full_heart_tex, icon_pos, icon_size);
            bool is_visible = (i < current_health); // 前景图标的可见性取决于当前生命值
            fg_icon->setVisible(is_visible);

            health_panel_->addChild(std::move(fg_icon));
        }

        // 检查UI管理器是否存在
        if (!ui_manager_) {
            spdlog::error("UI管理器不存在，无法添加生命值面板");
            return;
        }

        // 将UIPanel添加到UI管理器中
        ui_manager_->addElement(std::move(health_panel));

        spdlog::trace("生命值UI创建完成");
    }

    void GameScene::addScoreWithUI(int score)
    {
        game_session_data_->addScore(score);
        auto score_text = "Score: " + std::to_string(game_session_data_->getCurrentScore());
        spdlog::info("得分: {}", score_text);
        score_label_->setText(score_text);
    }

    void GameScene::healWithUI(int amount)
    {
        player_->getComponent<engine::component::HealthComponent>()->heal(amount);
        updateHealthWithUI(); // 更新生命值与UI
    }

    void GameScene::updateHealthWithUI()
    {
        if (!player_ || !health_panel_) {
            spdlog::error("玩家对象或 HealthPanel 不存在，无法更新生命值UI");
            return;
        }

        // 获取当前生命值并更新游戏数据
        auto current_health =
            player_->getComponent<engine::component::HealthComponent>()->getCurrentHealth();
        game_session_data_->setCurrentHealth(current_health);
        auto max_health = game_session_data_->getMaxHealth();

        // 前景图标是后添加的，因此设置后半段的可见性即可
        for (auto i = max_health; i < max_health * 2; ++i) {
            health_panel_->getChildren()[i]->setVisible(i - max_health < current_health);
        }
    }

} // namespace game::scene
