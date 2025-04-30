#pragma once
#include "component_manager.h"
#include "camera.h"

constexpr int MAX_ACTIVE_PROBES = 8;
constexpr Entity INVALID_ENTITY = INT16_MAX;

class ProbeSystem
{
private:
	std::vector<Entity> cachedActiveProbes;

public:
	void RebuildProbes(
		SceneEntityRegistry& sceneRegistry,
		EnvironmentProbeManager& probeManager)
	{
		for (Entity entity : sceneRegistry.GetAll())
		{
			auto* probeComp = probeManager.GetComponent(entity);
			if (!probeComp || !probeComp->buildProbe) continue;

			// destroy current maps
			if (probeComp->maps.envMap)
			{
				IBLGenerator::Destroy(probeComp->maps);
				probeComp->maps = {};
			}

			// build new IBL maps
			probeComp->maps = IBLGenerator::Build(probeComp->settings);

			probeComp->buildProbe = false;
		}
	}

	std::vector<Entity> GetActiveProbes(
		SceneEntityRegistry& sceneRegistry,
		EnvironmentProbeManager& probeManager,
		Camera& camera)
	{
		static glm::vec3 lastCamPos = camera.getCameraPos();
		static float cacheTresholdSq = 0.5f * 0.5f;
		glm::vec3 d = camera.getCameraPos() - lastCamPos;

		if (glm::dot(d,d) < cacheTresholdSq && !cachedActiveProbes.empty()) 
			return cachedActiveProbes;

		lastCamPos = camera.getCameraPos();

		struct ProbeDist
		{
			Entity entity;
			float distance2;
		};

		Entity skyboxEnt = INVALID_ENTITY;
		std::vector<ProbeDist> probes;
		
		for (Entity entity : sceneRegistry.GetAll())
		{
			auto* probeComp = probeManager.GetComponent(entity);
			if (!probeComp) continue;
			
			// check if it's the skybox entity
			if (probeComp->radius == std::numeric_limits<float>::infinity() && skyboxEnt == INVALID_ENTITY)
			{
				skyboxEnt = entity;
				continue;
			}
			
			// get distance squared
			glm::vec3 pos = probeComp->position;
			glm::vec3 diff = camera.getCameraPos() - pos;
			float dist2 = glm::dot(diff, diff);

			if (dist2 <= probeComp->radius * probeComp->radius)
				probes.push_back({ entity, dist2 });
		}

		// sort probes nearest to camera
		std::sort(probes.begin(), probes.end(),[](auto& a, auto& b)
			{
				return a.distance2 < b.distance2;
			});

		cachedActiveProbes.clear();
		for (int i = 0; i < probes.size() && i < MAX_ACTIVE_PROBES-1; ++i)
			cachedActiveProbes.push_back(probes[i].entity);

		// insert skybox as last probe
		if (skyboxEnt != INVALID_ENTITY) cachedActiveProbes.push_back(skyboxEnt);
		
		return cachedActiveProbes;
	}
};