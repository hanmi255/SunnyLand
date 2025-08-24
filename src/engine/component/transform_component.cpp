#include "transform_component.h"
#include "../object/game_object.h"
#include "collider_component.h"
#include "sprite_component.h"

namespace engine::component {

    void TransformComponent::setScale(glm::vec2 scale)
    {
        scale_ = scale;
        if (owner_ != nullptr) {
            auto* sprite_component = owner_->getComponent<SpriteComponent>();
            if (sprite_component != nullptr) {
                sprite_component->updateOffset();
            }

            auto* collider_component = owner_->getComponent<ColliderComponent>();
            if (collider_component != nullptr) {
                collider_component->updateOffset();
            }
        }
    }

} // namespace engine::component
