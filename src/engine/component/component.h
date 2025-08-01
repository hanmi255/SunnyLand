#pragma once

namespace engine::core {
    class Context;
} // namespace engine::core

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace engine::component {

    /**
     * @brief 组件的抽象基类。
     *
     * 所有具体组件都应从此类继承。
     * 定义了组件生命周期中可能调用的通用方法。
     */
    class Component {
        friend class engine::object::GameObject;

    protected:
        engine::object::GameObject* owner_ = nullptr;

    public:
        Component() = default;
        virtual ~Component() = default;

        // 禁止拷贝和移动语义
        Component(const Component &) = delete;
        Component &operator=(const Component &) = delete;
        Component(const Component &&) = delete;
        Component &operator=(const Component &&) = delete;

        // --- getter & setter ---
        engine::object::GameObject* getOwner() const { return owner_; }
        void setOwner(engine::object::GameObject* owner) { owner_ = owner; }

    protected:
        // 关键循环函数，全部设为保护，只有 GameObject 需要（可以）调用
        virtual void init() {}
        virtual void handleInput(engine::core::Context &) {}
        virtual void update(double, engine::core::Context &) {}
        virtual void render(engine::core::Context &) {}
        virtual void clean() {}
    };
} // namespace engine::component
