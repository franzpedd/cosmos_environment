#include "scene/entity.h"

#include "scene/components.h"

namespace Cosmos
{
    Entity::Entity(World* world, entt::entity handle)
        : mWorld(world), mHandle(handle)
    {
    }

    void Entity::Serialize(Datafile& data)
    {
        IDComponent::Serialize(this, data);
        NameComponent::Serialize(this, data);
    }
}