#pragma once
#include "player_state.h"

namespace game::component::player_state {
    class ClimbState final : public PlayerState {
        friend class game::component::PlayerComponent;

    public:
        explicit ClimbState(PlayerComponent* player_component) : PlayerState(player_component) {}
        ~ClimbState() override = default;
        [[nodiscard]] const char* getStateName() const override { return "ClimbState"; }

    private:
        void enter() override;
        void exit() override;
        std::unique_ptr<PlayerState> handleInput(engine::core::Context & /*unused*/) override;
        std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context & /*unused*/) override;
    };
} // namespace game::component::player_state