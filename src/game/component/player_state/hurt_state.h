#pragma once
#include "player_state.h"

namespace game::component::player_state {

    class HurtState final : public PlayerState {
        friend class game::component::PlayerComponent;

    private:
        float stunned_timer_ = 0.0F; ///< @brief 硬直计时器，单位为秒

    public:
        explicit HurtState(PlayerComponent* player_component) : PlayerState(player_component) {}
        ~HurtState() override = default;
        [[nodiscard]] const char* getStateName() const override { return "HurtState"; }

    private:
        void enter() override;
        void exit() override;
        std::unique_ptr<PlayerState> handleInput(engine::core::Context & /*unused*/) override;
        std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context & /*unused*/) override;
    };

} // namespace game::component::player_state
