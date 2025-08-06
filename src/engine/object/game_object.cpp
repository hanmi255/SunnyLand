#include "game_object.h"
#include "../input/input_manager.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include <spdlog/spdlog.h>

namespace engine::object {
    GameObject::GameObject(const std::string_view &name, const std::string_view &tag)
        : name_(name), tag_(tag)
    {
        spdlog::trace("GameObject created: {} {}", name, tag);
    }

    void GameObject::update(float delta_time, engine::core::Context &context)
    {
        // 遍历所有组件并更新
        for (const auto &pair : components_) {
            pair.second->update(delta_time, context);
        }
    }

    void GameObject::render(engine::core::Context &context)
    {
        // 遍历所有组件并渲染
        for (const auto &pair : components_) {
            pair.second->render(context);
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

    void GameObject::handleInput(engine::core::Context &context)
    {
        // 遍历所有组件并处理输入
        for (const auto &pair : components_) {
            pair.second->handleInput(context);
        }
    }
} // namespace engine::object
