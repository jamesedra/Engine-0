#pragma once
#include "worldcomponents.h"
#include "entity_manager.h"
#include <optional>

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

class MaterialsGroupManager
{
public:
	std::unordered_map<Entity, MaterialsGroupComponent> components;
	MaterialsGroupComponent* GetComponent(Entity entity)
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

class EnvironmentProbeManager
{
public:
	std::unordered_map<Entity, EnvironmentProbeComponent> probeComponents;
	std::optional<std::pair<Entity, EnvironmentProbeComponent>> skyProbeComponent;

	EnvironmentProbeComponent* GetProbeComponent(Entity entity)
	{
		auto it = probeComponents.find(entity);
		return (it != probeComponents.end()) ? &it->second : nullptr;
	}

	void AddSkyProbe(Entity e, const EnvironmentProbeComponent& sky)
	{
		if (skyProbeComponent.has_value()) RemoveSkyProbe();
		skyProbeComponent = { e, sky };
	}

	void RemoveSkyProbe()
	{
		skyProbeComponent.reset();
	}

	EnvironmentProbeComponent* GetSkyProbe()
	{
		return skyProbeComponent ? &skyProbeComponent->second : nullptr;
	}
};

class LightManager
{
public:
	std::unordered_map<Entity, PointLightComponent> pointLightComponents;
	std::unordered_map<Entity, DirectionalLightComponent> directionalLightComponents;

	PointLightComponent* GetPointLightComponent(Entity entity)
	{
		auto it = pointLightComponents.find(entity);
		return (it != pointLightComponents.end()) ? &it->second : nullptr;
	}

	DirectionalLightComponent* GetDirectionalLightComponent(Entity entity)
	{
		auto it = directionalLightComponents.find(entity);
		return (it != directionalLightComponents.end()) ? &it->second : nullptr;
	}

	std::optional<std::pair<Entity, DirectionalLightComponent*>> GetAnyDirectionalLight()
	{
		if (directionalLightComponents.empty()) return std::nullopt;
		auto it = directionalLightComponents.begin();
		return std::make_pair(it->first, &it->second);
	}

	void RemoveDirectionalLights()
	{
		directionalLightComponents.clear();
	}
};

class LandscapeManager
{
	public:
		std::unordered_map<Entity, LandscapeComponent> landscapeComponents;
		std::unordered_map<Entity, HeightGenComponent> heightGenComponents;

		LandscapeComponent* GetLandscapeComponent(Entity entity)
		{
			auto it = landscapeComponents.find(entity);
			return (it != landscapeComponents.end()) ? &it->second : nullptr;
		}

		HeightGenComponent* GetHeightGenComponent(Entity entity)
		{
			auto it = heightGenComponents.find(entity);
			return (it != heightGenComponents.end()) ? &it->second : nullptr;
		}
};

