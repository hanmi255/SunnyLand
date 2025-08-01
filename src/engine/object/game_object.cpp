#include "game_object.h"
#include "../input/input_manager.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include <spdlog/spdlog.h>

namespace engine::object {
    GameObject::GameObject(const std::string &name, const std::string &tag) : name_(name), tag_(tag)
    {
        spdlog::trace("GameObject created: {} {}", name, tag);
    }

    void GameObject::update(double delta_time)
    {
        // 遍历所有组件并更新
        for (const auto &pair : components_) {
            pair.second->update(delta_time);
        }
    }

    void GameObject::render()
    {
        // 遍历所有组件并渲染
        for (const auto &pair : components_) {
            pair.second->render();
        }
    }

    void GameObject::clean()
    {
        spdlog::trace("Cleaning GameObject...");
        // 遍历所有组件并清理
        for (const auto &pair : components_) {
            pair.second->clean();
        }
        components_.clear();
    }

    void GameObject::handleInput()
    {
        // 遍历所有组件并处理输入
        for (const auto &pair : components_) {
            pair.second->handleInput();
        }
    }
} // namespace engine::object
