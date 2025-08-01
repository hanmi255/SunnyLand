#pragma once
#include "../component/component.h"
#include <memory> // 用于 std::unique_ptr
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <typeindex> // 用于 std::type_index
#include <unordered_map>
#include <utility>   // 用于 std::forward

namespace engine::object {

    class GameObject final {
    private:
        std::string name_;
        std::string tag_;
        std::unordered_map<std::type_index, std::unique_ptr<component::Component>> components_;
        bool need_remove_ = false; ///<@breif 将来由场景类负责删除

    public:
        GameObject(const std::string &name = "", const std::string &tag = "");

        // 禁止拷贝和移动语义
        GameObject(const GameObject &) = delete;
        GameObject &operator=(const GameObject &) = delete;
        GameObject(const GameObject &&) = delete;
        GameObject &operator=(const GameObject &&) = delete;

        // --- getters ---
        const std::string_view &getName() const { return name_; }
        const std::string_view &getTag() const { return tag_; }
        bool isNeedRemove() const { return need_remove_; }

        // --- setters ---
        void setName(const std::string_view &name) { name_ = name; }
        void setTag(const std::string_view &tag) { tag_ = tag; }
        void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }

        // 关键循环函数
        void update(double delta_time);
        void render();
        void clean();
        void handleInput();

    public:
        /**
         * @brief 添加组件 (里面会完成组件的init())
         *
         * @tparam T 组件类型
         * @tparam Args 组件构造函数参数类型
         * @param args 组件构造函数参数
         * @return 组件指针，如果组件已存在则返回现有组件
         */
        template <typename T, typename... Args> T* addComponent(Args &&... args)
        {
            static_assert(std::is_base_of_v<engine::component::Component, T>,
                          "T 必须继承自 Component");

            // 如果组件已经存在，直接返回现有组件
            if (T* existing = getComponent<T>()) {
                spdlog::debug("GameObject::addComponent: {} component {} already exists", name_,
                              typeid(T).name());
                return existing;
            }

            try {
                // 创建新组件
                auto new_component = std::make_unique<T>(std::forward<Args>(args)...);
                T* ptr = new_component.get();

                // 设置拥有者
                new_component->setOwner(this);

                // 先存储组件，再初始化（确保异常安全）
                auto type_index = getTypeIndex<T>();
                components_[type_index] = std::move(new_component);

                // 初始化组件
                ptr->init();

                spdlog::debug("GameObject::addComponent: {} added component {}", name_,
                              typeid(T).name());
                return ptr;

            } catch (const std::exception &e) {
                spdlog::error("GameObject::addComponent: Failed to add component {} to {}: {}",
                              typeid(T).name(), name_, e.what());
                throw;
            }
        }

        /**
         * @brief 获取组件
         *
         * @tparam T 组件类型
         * @return 组件指针，如果不存在则返回nullptr
         */
        template <typename T> T* getComponent() const noexcept
        {
            static_assert(std::is_base_of_v<engine::component::Component, T>,
                          "T 必须继承自 Component");

            if (auto it = components_.find(getTypeIndex<T>()); it != components_.end()) {
                return static_cast<T*>(it->second.get());
            }
            return nullptr;
        }

        /**
         * @brief 检查是否存在组件
         *
         * @tparam T 组件类型
         * @return 是否存在组件
         */
        template <typename T> bool hasComponent() const noexcept
        {
            static_assert(std::is_base_of_v<engine::component::Component, T>,
                          "T 必须继承自 Component");

            return components_.contains(getTypeIndex<T>());
        }

        /**
         * @brief 移除组件
         *
         * @tparam T 组件类型
         * @return 是否成功移除组件
         */
        template <typename T> bool removeComponent()
        {
            static_assert(std::is_base_of_v<engine::component::Component, T>,
                          "T 必须继承自 Component");

            auto type_index = getTypeIndex<T>();
            auto it = components_.find(type_index);

            if (it != components_.end()) {
                try {
                    // 先清理组件，再移除
                    it->second->clean();
                    components_.erase(it);

                    spdlog::debug("GameObject::removeComponent: {} removed component {}", name_,
                                  typeid(T).name());
                    return true;

                } catch (const std::exception &e) {
                    spdlog::error(
                        "GameObject::removeComponent: Failed to remove component {} from {}: {}",
                        typeid(T).name(), name_, e.what());
                    // 即使清理失败，也应该移除组件避免悬空指针
                    components_.erase(it);
                    throw;
                }
            }

            spdlog::debug("GameObject::removeComponent: {} component {} not found", name_,
                          typeid(T).name());
            return false;
        }

        /**
         * @brief 获取或添加组件（如果不存在则创建）
         *
         * @tparam T 组件类型
         * @tparam Args 组件构造函数参数类型
         * @param args 组件构造函数参数
         * @return 组件指针
         */
        template <typename T, typename... Args> T* getOrAddComponent(Args &&... args)
        {
            if (T* existing = getComponent<T>()) {
                return existing;
            }
            return addComponent<T>(std::forward<Args>(args)...);
        }

    private:
        // 辅助函数：获取类型索引（避免重复代码）
        template <typename T> static constexpr std::type_index getTypeIndex()
        {
            return std::type_index(typeid(T));
        }
    };
} // namespace engine::object
