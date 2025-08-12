#include "scene.h"
#include "../core/context.h"
#include "../core/game_state.h"
#include "../object/game_object.h"
#include "../physics/physics_engine.h"
#include "../render/camera.h"
#include "../ui/ui_manager.h"
#include "scene_manager.h"
#include <algorithm> // 用于 std::remove_if
#include <memory>
#include <spdlog/spdlog.h>

namespace engine::scene {
    Scene::Scene(std::string_view name, engine::core::Context &context,
                 engine::scene::SceneManager &scene_manager)
        : name_(name)
        , context_(context)
        , scene_manager_(scene_manager)
        , ui_manager_(std::make_unique<engine::ui::UIManager>())
        , is_initialized_(false)
    {
        spdlog::trace("场景 '{}' 构建成功", name_);
    }

    Scene::~Scene() = default;

    void Scene::init()
    {
        is_initialized_ = true;
        spdlog::trace("场景 '{}' 初始化完成", name_);
    }

    void Scene::update(float delta_time)
    {
        if (!is_initialized_) return;

        // 只有游戏进行中，才需要更新物理引擎和相机
        if (!context_.getGameState().isPlaying()) return;

        context_.getPhysicsEngine().update(delta_time);
        context_.getCamera().update(delta_time);

        // 遍历所有游戏对象并更新
        auto remove_predicate = [delta_time, this](auto &object) -> bool {
            if (!object) return true;

            if (object->isNeedRemove()) {
                object->clean();
                return true;
            }

            object->update(delta_time, context_);
            return false;
        };

        // 移除需要删除的对象
        game_objects_.erase(
            std::remove_if(game_objects_.begin(), game_objects_.end(), remove_predicate),
            game_objects_.end());

        // 更新 UI
        ui_manager_->update(delta_time, context_);
        processPendingActions();
    }

    void Scene::render()
    {
        if (!is_initialized_) return;
        for (auto const &object : game_objects_) {
            if (object) object->render(context_);
        }

        // 渲染 UI
        ui_manager_->render(context_);
    }

    void Scene::handleInput()
    {
        if (!is_initialized_) return;

        // 如果被 UI 元素处理，则不处理游戏对象
        if (ui_manager_->handleInput(context_)) return;

        auto handle_input_predicate = [this](auto &object) -> bool {
            if (!object) return true;

            if (object->isNeedRemove()) {
                object->clean();
                return true;
            }

            object->handleInput(context_);
            return false;
        };

        // 处理输入并移除需要删除的对象
        game_objects_.erase(
            std::remove_if(game_objects_.begin(), game_objects_.end(), handle_input_predicate),
            game_objects_.end());
    }

    void Scene::clean()
    {
        if (!is_initialized_) return;
        for (const auto &object : game_objects_) {
            if (object) object->clean();
        }
        game_objects_.clear();
        is_initialized_ = false;
        spdlog::trace("场景 '{}' 已清理", name_);
    }

    void Scene::addGameObject(std::unique_ptr<engine::object::GameObject> &&object)
    {
        if (object)
            game_objects_.push_back(std::move(object));
        else
            spdlog::warn("尝试向场景 '{}' 添加空游戏对象。", name_);
    }

    void Scene::safelyAddGameObject(std::unique_ptr<engine::object::GameObject> &&object)
    {
        if (object)
            pending_additions_.push_back(std::move(object));
        else
            spdlog::warn("尝试向场景 '{}' 添加空游戏对象。", name_);
    }

    void Scene::removeGameObject(engine::object::GameObject* object_ptr)
    {
        if (!object_ptr) {
            spdlog::warn("尝试从场景 '{}' 中移除空的游戏对象指针", name_);
            return;
        }

        if (game_objects_.empty()) {
            spdlog::warn("场景 '{}' 中没有游戏对象可移除", name_);
            return;
        }

        // 使用find_if查找第一个匹配的对象（更高效）
        auto it = std::find_if(
            game_objects_.begin(), game_objects_.end(),
            [object_ptr](const auto &ptr) noexcept { return ptr && ptr.get() == object_ptr; });

        if (it == game_objects_.end()) {
            spdlog::warn("游戏对象指针未在场景 '{}' 中找到", name_);
            return;
        }

        try {
            if (*it) {
                (*it)->clean();
            }
        } catch (const std::exception &e) {
            spdlog::error("清理游戏对象时发生异常: {}", e.what());
        }

        // 删除对象
        game_objects_.erase(it);
        spdlog::trace("从场景 '{}' 中成功移除游戏对象", name_);
    }

    void Scene::safelyRemoveGameObject(engine::object::GameObject* object_ptr)
    {
        object_ptr->setNeedRemove(true);
    }

    engine::object::GameObject* Scene::findGameObjectByName(std::string_view name) const
    {
        for (const auto &object : game_objects_) {
            if (object && object->getName() == name) {
                return object.get();
            }
        }
        return nullptr;
    }

    void Scene::processPendingActions()
    {
        for (auto &object : pending_additions_) {
            addGameObject(std::move(object));
        }
        pending_additions_.clear();
    }

} // namespace engine::scene