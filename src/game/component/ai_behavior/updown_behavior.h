#pragma once
#include "ai_behavior.h"

namespace game::component::ai_behavior {

    /**
     * @brief AI 行为：在指定范围内上下垂直移动。
     *
     * 到达边界或碰到障碍物时会反向。
     */
    class UpDownBehavior final : public AIBehavior {
        friend class game::component::AIComponent;

    private:
        float patrol_min_y_ = 0.0f; ///< @brief 巡逻范围的上边界 (Y 坐标较小值)
        float patrol_max_y_ = 0.0f; ///< @brief 巡逻范围的下边界 (Y 坐标较大值)
        float move_speed_ = 50.0f;  ///< @brief 移动速度 (像素/秒)
        bool moving_down_ = false;  ///< @brief 当前是否向下移动

    public:
        UpDownBehavior(float min_y, float max_y, float speed = 50.0f);
        ~UpDownBehavior() override = default;

        // 禁止拷贝和移动语义
        UpDownBehavior(const UpDownBehavior &) = delete;
        UpDownBehavior &operator=(const UpDownBehavior &) = delete;
        UpDownBehavior(UpDownBehavior &&) = delete;
        UpDownBehavior &operator=(UpDownBehavior &&) = delete;

    private:
        void enter(AIComponent &ai_component) override;
        void update(float delta_time, AIComponent &ai_component) override;
    };

} // namespace game::component::ai_behavior
