#pragma once

#include "util/library.h"

#include <entt.hpp>

// forward declarations
namespace Cosmos { class Entity; }

namespace Cosmos
{
    class World
    {
    public:

        /// @brief constructor
        World(const char* name);

        /// @brief destructor
        ~World();

        /// @brief returns a reference to the world's entities
        inline Library<Entity*>& GetEntityLibraryRef() { return mEntities; }

        /// @brief returns a reference to the world entity registry
        inline entt::registry& GetRegistryRef() { return mRegistry; }

    public:

        /// called once per frame, updates the world
        void OnUpdate(double timestep);

        /// called once per frame, renders the world
        void OnRender(int stage, double interpolation);

    private:

        const char* mName;
        Library<Entity*> mEntities;
        entt::registry mRegistry;
    };
}