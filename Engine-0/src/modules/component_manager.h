#pragma once
#include "worldcomponents.h"

// NOTE: this is mostly for holding data sets (currently uses a map, with the entity as the key) of components for preparing the components that is needed for a certain system.

using Entity = unsigned int;

class TransformManager
{
public:
	std::unordered_map<Entity, TransformComponent> components;
	TransformComponent* GetComponent(Entity entity)
	{
		auto it = components.find(entity);
		return (it != components.end()) ? &it->second : nullptr;
	}
};

class MeshManager
{
public:
	std::unordered_map<Entity, MeshComponent> components;
	MeshComponent* GetComponent(Entity entity)
	{
		auto it = components.find(entity);
		return (it != components.end()) ? &it->second : nullptr;
	}
};

class ShaderManager
{
public:
	std::unordered_map<Entity, ShaderComponent> components;
	ShaderComponent* GetComponent(Entity entity)
	{
		auto it = components.find(entity);
		return (it != components.end()) ? &it->second : nullptr;
	}
};

class MaterialManager
{
public:
	std::unordered_map<Entity, MaterialComponent> components;
	MaterialComponent* GetComponent(Entity entity)
	{
		auto it = components.find(entity);
		return (it != components.end()) ? &it->second : nullptr;
	}
};