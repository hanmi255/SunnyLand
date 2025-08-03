#include "transform_component.h"
#include "../object/game_object.h"
#include "sprite_component.h"
#include "collider_component.h"

namespace engine::component {

    void TransformComponent::setScale(const glm::vec2 &scale)
    {
        scale_ = scale;
        if (owner_) {
            auto sprite_component = owner_->getComponent<SpriteComponent>();
            if (sprite_component) {
                sprite_component->updateOffset();
            }

            auto collider_component = owner_->getComponent<ColliderComponent>();
            if (collider_component) {
                collider_component->updateOffset();
            }
        }
    }

} // namespace engine::component
