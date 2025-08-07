#include "scene_manager.h"
#include "../core/context.h"
#include "scene.h"
#include <spdlog/spdlog.h>

namespace engine::scene {
    SceneManager::SceneManager(engine::core::Context &context) : context_(context)
    {
        spdlog::trace("SceneManager 创建成功");
    }

    SceneManager::~SceneManager()
    {
        spdlog::trace("SceneManager 销毁成功");
        clean();
    }

    void SceneManager::update(float delta_time)
    {
        // 处理待处理的操作
        processPendingActions();

        // 只更新栈顶场景
        if (auto* current_scene = getCurrentScene(); current_scene) {
            current_scene->update(delta_time);
        }
    }

    void SceneManager::render()
    {
        // 渲染所有场景（从底部到顶部）
        for (const auto &scene : scenes_stack_) {
            if (scene) {
                scene->render();
            }
        }
    }

    void SceneManager::handleInput()
    {
        // 只处理栈顶场景的输入
        if (auto* current_scene = getCurrentScene(); current_scene) {
            current_scene->handleInput();
        }
    }

    void SceneManager::clean()
    {
        if (scenes_stack_.empty()) {
            spdlog::trace("场景栈已为空，无需清理");
            return;
        }

        spdlog::trace("正在关闭场景管理器并清理场景栈...");
        while (!scenes_stack_.empty()) {
            if (scenes_stack_.back()) {
                spdlog::trace("正在清理场景 '{}'...", scenes_stack_.back()->getName());
                scenes_stack_.back()->clean();
            }
            scenes_stack_.pop_back();
        }

        // 清理待处理的操作
        resetPendingAction();
    }

    void SceneManager::requestPushScene(std::unique_ptr<Scene> &&scene)
    {
        if (!scene) {
            spdlog::warn("请求压入空场景指针，操作被忽略");
            return;
        }

        if (pending_action_ != PendingAction::NONE) {
            spdlog::warn("已有待处理的场景操作，新的压入请求被忽略");
            return;
        }

        pending_action_ = PendingAction::PUSH;
        pending_scene_ = std::move(scene);
        spdlog::debug("场景压入请求已排队: '{}'", pending_scene_->getName());
    }

    void SceneManager::requestPopScene()
    {
        if (scenes_stack_.empty()) {
            spdlog::warn("场景栈为空，弹出请求被忽略");
            return;
        }

        if (pending_action_ != PendingAction::NONE) {
            spdlog::warn("已有待处理的场景操作，新的弹出请求被忽略");
            return;
        }

        pending_action_ = PendingAction::POP;
        spdlog::debug("场景弹出请求已排队");
    }

    void SceneManager::requestReplaceScene(std::unique_ptr<Scene> &&scene)
    {
        if (!scene) {
            spdlog::warn("请求替换为空场景指针，操作被忽略");
            return;
        }

        if (pending_action_ != PendingAction::NONE) {
            spdlog::warn("已有待处理的场景操作，新的替换请求被忽略");
            return;
        }

        pending_action_ = PendingAction::REPLACE;
        pending_scene_ = std::move(scene);
        spdlog::debug("场景替换请求已排队: '{}'", pending_scene_->getName());
    }

    void SceneManager::processPendingActions()
    {
        if (pending_action_ == PendingAction::NONE) {
            return;
        }

        switch (pending_action_) {
            case PendingAction::PUSH:
                pushScene(std::move(pending_scene_));
                break;
            case PendingAction::POP:
                popScene();
                break;
            case PendingAction::REPLACE:
                replaceScene(std::move(pending_scene_));
                break;
            case PendingAction::NONE:
            default:
                break;
        }

        // 重置待处理状态
        resetPendingAction();
    }

    void SceneManager::pushScene(std::unique_ptr<Scene> &&scene)
    {
        if (!scene) {
            spdlog::error("尝试将空指针压入场景栈");
            return;
        }

        spdlog::debug("将场景 '{}' 压入场景栈", scene->getName());

        // 初始化新场景
        if (!scene->isInitialized()) {
            try {
                scene->init();
            } catch (const std::exception &e) {
                spdlog::error("场景 '{}' 初始化失败: {}", scene->getName(), e.what());
                return;
            }
        }

        // 压入栈
        scenes_stack_.push_back(std::move(scene));
        spdlog::info("场景 '{}' 已成功压入栈，当前栈大小: {}", scenes_stack_.back()->getName(),
                     scenes_stack_.size());
    }

    void SceneManager::popScene()
    {
        if (scenes_stack_.empty()) {
            spdlog::warn("尝试从空的场景栈弹出场景");
            return;
        }

        const std::string scene_name = scenes_stack_.back()->getName();
        spdlog::debug("将场景 '{}' 弹出场景栈", scene_name);

        // 清理并弹出场景
        if (scenes_stack_.back()) {
            scenes_stack_.back()->clean();
        }
        scenes_stack_.pop_back();

        spdlog::info("场景 '{}' 已弹出栈，当前栈大小: {}", scene_name, scenes_stack_.size());
    }

    void SceneManager::replaceScene(std::unique_ptr<Scene> &&scene)
    {
        if (!scene) {
            spdlog::error("尝试用空场景替换当前场景");
            return;
        }

        const std::string new_scene_name = scene->getName();
        const std::string old_scene_name = scenes_stack_.empty() ? "无场景"
                                                                 : scenes_stack_.back()->getName();

        spdlog::debug("将当前场景 '{}' 替换为场景 '{}'", old_scene_name, new_scene_name);

        // 清理并移除场景栈中所有场景
        while (!scenes_stack_.empty()) {
            if (scenes_stack_.back()) {
                scenes_stack_.back()->clean();
            }
            scenes_stack_.pop_back();
        }

        // 初始化新场景
        if (!scene->isInitialized()) {
            try {
                scene->init();
            } catch (const std::exception &e) {
                spdlog::error("替换场景 '{}' 初始化失败: {}", new_scene_name, e.what());
                return;
            }
        }

        // 压入新场景
        scenes_stack_.push_back(std::move(scene));
        spdlog::info("场景替换完成: '{}' -> '{}'", old_scene_name, new_scene_name);
    }

    Scene* SceneManager::getCurrentScene() const
    {
        return scenes_stack_.empty() ? nullptr : scenes_stack_.back().get();
    }

    void SceneManager::resetPendingAction()
    {
        pending_action_ = PendingAction::NONE;
        pending_scene_.reset();
    }

} // namespace engine::scene
