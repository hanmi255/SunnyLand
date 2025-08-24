#pragma once
#include "player_state.h"

namespace game::component::player_state {

    class WalkState final : public PlayerState {
        friend class game::component::PlayerComponent;

    public:
        explicit WalkState(PlayerComponent* player_component) : PlayerState(player_component) {}
        ~WalkState() override = default;
        [[nodiscard]] const char* getStateName() const override { return "WalkState"; }

    private:
        void enter() override;
        void exit() override;
        std::unique_ptr<PlayerState> handleInput(engine::core::Context & /*unused*/) override;
        std::unique_ptr<PlayerState> update(float delta_time, engine::core::Context & /*unused*/) override;
    };

} // namespace game::component::player_state
