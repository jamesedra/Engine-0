#pragma once
#include "component_manager.h"
#include "camera.h"

constexpr int MAX_ACTIVE_PROBES = 8;

class ProbeSystem
{
public:
	void RebuildProbes(
		SceneEntityRegistry& sceneRegistry,
		EnvironmentProbeManager& probeManager,
		Shader& EQRToCubemap,
		Shader& IrradianceShader,
		Shader& PrefilterShader,
		Shader& IntegratedBRDF,
		unsigned int cubeVAO,
		unsigned int frameVAO)
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
			probeComp->maps = IBLGenerator::Build(
				probeComp->settings,
				EQRToCubemap,
				IrradianceShader,
				PrefilterShader,
				IntegratedBRDF,
				cubeVAO,
				frameVAO
			);

			probeComp->buildProbe = false;
		}
	}

	std::vector<Entity> GetActiveProbes(
		SceneEntityRegistry& sceneRegistry,
		EnvironmentProbeManager& probeManager,
		Entity skyboxEntity, // a bit dirty
		Camera& camera)
	{
		struct ProbeDist
		{
			Entity entity;
			float distance2;
		};

		std::vector<ProbeDist> probes;

		for (Entity entity : sceneRegistry.GetAll())
		{
			auto* probeComp = probeManager.GetComponent(entity);
			if (!probeComp) continue;
			
			glm::vec3 pos = probeComp->position;

			float dist2 = glm::length(camera.getCameraPos() - pos);
			dist2 *= dist2;
			if (dist2 <= probeComp->radius * probeComp->radius 
				&& probeComp->radius < std::numeric_limits<float>::infinity()) // avoid skybox probe
				probes.push_back({ entity, dist2 });
		}

		std::sort(probes.begin(), probes.end(),[](auto& a, auto& b)
			{
				return a.distance2 < b.distance2;
			});

		std::vector<Entity> activeProbes;
		for (int i = 0; i < probes.size() && i < MAX_ACTIVE_PROBES-1; ++i)
			activeProbes.push_back(probes[i].entity);
		activeProbes.push_back(skyboxEntity); // insert skybox as last probe

		return activeProbes;
	}
};