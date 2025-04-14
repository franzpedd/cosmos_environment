#pragma once

#include "util/datafile.h"

#include <map>
#include <string>

// forward declarations
namespace Cosmos { class Entity; }
namespace Cosmos { class World; }

namespace Cosmos
{
    class Prefab
    {
    public:

        /// @brief creates an empty prefab without a name
        Prefab(World* world, std::string name = "New Prefab");

        // @brief creates a prefab with a previous assigned id
        Prefab(World* world, unsigned long long id, std::string name);

        /// @brief destroy and releases all used resources
        ~Prefab() = default;

        /// @brief returns the world this prefab belongs to
        inline World* GetWorld() { return mWorld; }

        /// @brief returns a referecen to the prefab name
        inline std::string GetNameRef() { return mName; }

        /// @brief returns this prefab's id value
        inline unsigned long long GetID() const { return mID; }

        /// @brief returns a reference to the children of this prefab
        inline std::multimap<std::string, Prefab*>& GetChildrenRef() { return mChildren; }

        /// @brief returns a reference to the entities of this prefab
        inline std::multimap<std::string, Entity*>& GetEntitiesRef() { return mEntities; }

    public:

        /// @brief adds a child of this prefab, a sub-prefab
        void InsertChild(std::string name);

        /// @brief removes and destroys a child prefab, and it's children-tree if any
        void EraseChild(Prefab* prefab, bool eraseFromMultimap = true);

        /// @brief adds an entity to this prefab
        void InsertEntity(std::string name);

        /// @brief removes and destroys a given entity
        void EraseEntity(Entity* entity, bool eraseFromMultimap = true);

        /// @brief duplicates a given entity into this prefab
        void DuplicateEntity(Entity* entity, bool considerOtherGroups = true);

    public:

        /// @brief saves the prefab entities and sub-prefabs into the datafile
        static void Serialize(Prefab* prefab, Datafile& world);

        /// @brief loads the prefab entities and sub-prefabs given the datafile
        static void Deserialize(Prefab* prefab, World* world, Datafile& worldData);

    private:

        /// @brief recursively erases and free, used internally
        static void Recursively_Delete(Prefab* current);

    private:

        World* mWorld = nullptr;
        std::string mName = {};
        unsigned long long mID = 0;
        
        std::multimap<std::string, Prefab*> mChildren = {};
        std::multimap<std::string, Entity*> mEntities = {};
    };
}