#include "physics_component.h"
#include "../object/game_object.h"
#include "../physics/physics_engine.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>

engine::component::PhysicsComponent::PhysicsComponent(
    engine::physics::PhysicsEngine* physics_engine, bool use_gravity, float mass)
    : physics_engine_(physics_engine), mass_(mass >= 0.0F ? mass : 1.0F), use_gravity_(use_gravity)
{
    if (physics_engine_ == nullptr) {
        spdlog::error("PhysicsComponent 构造函数中，PhysicsEngine 指针不能为nullptr！");
    }
    spdlog::trace("PhysicsComponent 创建完成，质量: {}, 使用重力: {}", mass_, use_gravity_);
}

void engine::component::PhysicsComponent::init()
{
    if (owner_ == nullptr) {
        spdlog::error("PhysicsComponent 在初始化前未设置 owner_。");
        return;
    }
    transform_component_ = owner_->getComponent<TransformComponent>();
    if (transform_component_ == nullptr) {
        spdlog::warn(
            "GameObject '{}' 上的 PhysicsComponent 需要一个 TransformComponent，但未找到。",
            owner_->getName());
        return;
    }
    if (physics_engine_ == nullptr) {
        spdlog::error("PhysicsComponent 初始化时，PhysicsEngine 未正确初始化。");
        return;
    }
    physics_engine_->registerComponent(this);
    spdlog::trace("PhysicsComponent 初始化完成。");
}

void engine::component::PhysicsComponent::clean()
{
    physics_engine_->unregisterComponent(this);
    spdlog::trace("PhysicsComponent 清理完成。");
}
