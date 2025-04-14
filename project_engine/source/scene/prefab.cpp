#include "scene/prefab.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scene/world.h"
#include "core/logger.h"

namespace Cosmos
{
    Prefab::Prefab(World* world, std::string name)
        : mWorld(world), mName(name)
    {
        COSMOS_LOG(LogSeverity::Todo, "Create unique id");
    }

    Prefab::Prefab(World* world, unsigned long long id, std::string name)
        : mWorld(world), mName(name), mID(id)
    {
    }

    void Prefab::InsertChild(std::string name)
    {
        mChildren.insert({ "New Prefab", new Prefab(mWorld, name) });
    }

    void Prefab::EraseChild(Prefab* prefab, bool eraseFromMultimap)
    {
        // remove the prefab from the list of prefabs
        bool found = false;
        auto range = mChildren.equal_range(prefab->GetNameRef());
        for (auto& it = range.first; it != range.second; ++it) {
            if (it->second == prefab) {
                if (eraseFromMultimap) {
                    mChildren.erase(it);
                }
                found = true;
                break;
            }
        }

        if (!found) {
            COSMOS_LOG(LogSeverity::Error, "Could not find the given entity to destroy it");
            return;
        }

        // delete prefab and it's children
        Recursively_Delete(prefab);
    }

    void Prefab::InsertEntity(std::string name)
    {
        entt::entity handle = mWorld->GetRegistryRef().create();
        Entity* entity = new Entity(mWorld, handle);

        // create identifiers
        entity->AddComponent<IDComponent>();
        entity->AddComponent<NameComponent>(name);

        // insert into the prefab entity list
        mEntities.insert({ name, entity });

        // insert into the scene entity library
        std::stringstream ss;
        ss << entity->GetComponent<IDComponent>().id;
        mWorld->GetEntityLibraryRef().Insert(ss.str(), entity);
    }

    void Prefab::EraseEntity(Entity* entity, bool eraseFromMultimap)
    {
        bool found = false;
        auto range = mEntities.equal_range(entity->GetComponent<NameComponent>().name);
        for (auto& it = range.first; it != range.second; ++it) {
            if (it->second == entity) {
                if (eraseFromMultimap) {
                    mEntities.erase(it);
                }
                found = true;
                break;
            }
        }

        if (!found) {
            COSMOS_LOG(LogSeverity::Error, "Could not find the given entity to destroy it");
            return;
        }

        // remove from the scene entity library
        std::stringstream ss;
        ss << entity->GetComponent<IDComponent>().id;
        mWorld->GetEntityLibraryRef().Remove(ss.str());

        entity->RemoveComponent<NameComponent>();
        entity->RemoveComponent<IDComponent>();

        // delete from the prefab list of entities
        delete entity;
    }

    void Prefab::DuplicateEntity(Entity* entity, bool considerOtherGroups)
    {
        if (!entity) {
            return;
        }

        bool found = false;
        if (!considerOtherGroups) {
            auto range = mEntities.equal_range(entity->GetComponent<NameComponent>().name);
            for (auto& it = range.first; it != range.second; ++it) {
                if (it->second == entity) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                COSMOS_LOG(LogSeverity::Error, "Cannot duplicate entities from other groups without setting it the flag");
                return;
            }
        }

        // create a new entity with a new uuid
        entt::entity handle = mWorld->GetRegistryRef().create();
        Entity* newEntity = new Entity(mWorld, handle);
        newEntity->AddComponent<IDComponent>();

        // create all components based on the other entity ones
        if (entity->HasComponent<NameComponent>()) {
            newEntity->AddComponent<NameComponent>(entity->GetComponent<NameComponent>().name);
        }

        if (entity->HasComponent<TransformComponent>()) {
            newEntity->AddComponent<TransformComponent>(
                entity->GetComponent<TransformComponent>().translation,
                entity->GetComponent<TransformComponent>().rotation,
                entity->GetComponent<TransformComponent>().scale
            );
        }

        // insert into the library of entities
        std::stringstream ss;
        ss << newEntity->GetComponent<IDComponent>().id;
        mWorld->GetEntityLibraryRef().Insert(ss.str(), newEntity);

        mEntities.insert({ newEntity->GetComponent<NameComponent>().name, newEntity });
    }

    void Prefab::Serialize(Prefab* prefab, Datafile& sceneData)
    {
        std::string id = "Prefab:";
        id.append(std::to_string(prefab->GetID()));

        sceneData[id]["Name"].SetString(prefab->GetNameRef());
        sceneData[id]["Id"].SetString(std::to_string(prefab->GetID()));

        for (auto& child : prefab->GetChildrenRef()) {
            child.second->Serialize(child.second, sceneData[id]["Prefabs"]);
        }

        for (auto& entity : prefab->GetEntitiesRef()) {
            entity.second->Serialize(sceneData[id]["Entities"]);
        }
    }

    void Prefab::Deserialize(Prefab* prefab, World* scene, Datafile& sceneData)
    {
        if (sceneData.Exists("Prefabs")) {
            for (size_t i = 0; i < sceneData["Prefabs"].GetChildrenCount(); i++) {
                Datafile prefabData = sceneData["Prefabs"][i];
                std::string name = prefabData["Name"].GetString();
                std::string id = prefabData["Id"].GetString();

                Prefab* child = new Prefab(scene, std::stoull(id), name);
                prefab->GetChildrenRef().insert({ name, child });

                Deserialize(child, scene, prefabData);
            }
        }

        if (sceneData.Exists("Entities")) {
            for (size_t i = 0; i < sceneData["Entities"].GetChildrenCount(); i++) {
                Datafile data = sceneData["Entities"][i];
                entt::entity handle = scene->GetRegistryRef().create();
                Entity* entity = new Entity(scene, handle);

                IDComponent::Deserialize(entity, data);
                NameComponent::Deserialize(entity, data);
                TransformComponent::Deserialize(entity, data);

                prefab->GetEntitiesRef().insert({ entity->GetComponent<NameComponent>().name, entity });

                // insert into the scene entity library
                std::stringstream ss;
                ss << entity->GetComponent<IDComponent>().id;
                prefab->GetWorld()->GetEntityLibraryRef().Insert(ss.str(), entity);
            }
        }
    }

    void Prefab::Recursively_Delete(Prefab* current)
    {
        for (auto& entity : current->GetEntitiesRef()) {
            current->EraseEntity(entity.second, false);
        }

        for (auto& child : current->GetChildrenRef()) {
            Recursively_Delete(child.second);
        }

        current->GetEntitiesRef().clear();
        current->GetChildrenRef().clear();

        delete current;
    }
}