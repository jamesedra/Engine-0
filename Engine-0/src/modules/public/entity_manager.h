#pragma once
#include <unordered_set>

using Entity = unsigned int;

// Entity manager is mainly for creating a unique ID
class EntityManager
{
public:
    EntityManager() : nextEntity(0) {}

    Entity CreateEntity()
    {
        return nextEntity++;
    }

private:
    Entity nextEntity;
};

class SceneEntityRegistry
{
public:
    void Register(Entity entity)
    {
        sceneEntities.insert(entity);
    }

    bool Contains(Entity entity) const
    {
        return sceneEntities.find(entity) != sceneEntities.end();
    }

    const std::unordered_set<Entity>& GetAll() const
    {
        return sceneEntities;
    }

private:
    std::unordered_set<Entity> sceneEntities;
};