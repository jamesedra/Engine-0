#pragma once

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