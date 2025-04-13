#pragma once
#include "worldcomponents.h"
#include "entity_manager.h"

// NOTE: this is mostly for holding data sets (currently uses a map, with the entity as the key) of components for preparing the components that is needed for a certain system.

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

class IDManager
{
public:
	std::unordered_map<Entity, IDComponent> components;
	IDComponent* GetComponent(Entity entity)
	{
		auto it = components.find(entity);
		return (it != components.end()) ? &it->second : nullptr;
	}
};

class AssetManager
{
public:
	std::unordered_map<Entity, AssetComponent> components;
	AssetComponent* GetComponent(Entity entity)
	{
		auto it = components.find(entity);
		return (it != components.end()) ? &it->second : nullptr;
	}
};

class MaterialsManager
{
public:
	std::unordered_map<Entity, MaterialsComponent> components;
	MaterialsComponent* GetComponent(Entity entity)
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

// to be deprecated
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

