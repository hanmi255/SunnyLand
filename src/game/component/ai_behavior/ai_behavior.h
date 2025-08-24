#pragma once

namespace game::component {
    class AIComponent;
} // namespace game::component

namespace game::component::ai_behavior {

    /**
     * @brief AI 行为策略的抽象基类。
     */
    class AIBehavior {
        friend class game::component::AIComponent;

    public:
        AIBehavior() = default;
        virtual ~AIBehavior() = default;

        // 禁止移动和拷贝语义
        AIBehavior(const AIBehavior &) = delete;
        AIBehavior &operator=(const AIBehavior &) = delete;
        AIBehavior(AIBehavior &&) = delete;
        AIBehavior &operator=(AIBehavior &&) = delete;

    protected:
        virtual void enter(AIComponent & /*unused*/) {}
        virtual void update(float, AIComponent &) = 0;
    };
} // namespace game::component::ai_behavior
