#pragma once
#include "player_state.h"

namespace game::component::player_state {

    class FallState final : public PlayerState {
        friend class game::component::PlayerComponent;

    public:
        explicit FallState(PlayerComponent* player_component) : PlayerState(player_component) {}
        ~FallState() override = default;
        [[nodiscard]] const char* getStateName() const override { return "FallState"; }

    private:
        void enter() override;
        void exit() override;
        std::unique_ptr<PlayerState> handleInput(engine::core::Context & /*unused*/) override;
        std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context & /*unused*/) override;
    };

} // namespace game::component::player_state
