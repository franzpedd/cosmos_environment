#pragma once

#include <entt.hpp>
#include "scene/world.h"
#include "util/datafile.h"

namespace Cosmos
{
    class Entity
    {
    public:

        /// @brief constructor
        Entity(World* world, entt::entity handle);

        /// @brief destructor
        ~Entity() = default;

        /// @brief returns the entity unique handle
        inline entt::entity GetHandle() const { return mHandle; }

    public:

        /// @brief adds/replaces a component to the entity
        template<typename T, typename...Args>
        T& AddComponent(Args&&... args)
        {
            return mWorld->GetRegistryRef().emplace_or_replace<T>(mHandle, std::forward<Args>(args)...);
        }

        /// @brief checks if the entity has a T component
        template<typename T>
        inline bool HasComponent()
        {
            return mWorld->GetRegistryRef().all_of<T>(mHandle);
        }

        /// @brief returns the
        template<typename T>
        inline T& GetComponent()
        {
            return mWorld->GetRegistryRef().get<T>(mHandle);
        }

        /// @brief removes the component if existent
        template<typename T>
        inline void RemoveComponent()
        {
            mWorld->GetRegistryRef().remove<T>(mHandle);
        }

    public:

        /// @brief serializes the entity and it's components into a datafile
        /// @param data the serialized data output file
        void Serialize(Datafile& data);

    private:

        World* mWorld;
        entt::entity mHandle;
    };
}