#pragma once
#include "component_manager.h"
#include "camera.h"

class RenderSystem
{
public:
	void Render(
		SceneEntityRegistry& sceneRegistry,
		TransformManager& transformManager,
		MeshManager& meshManager,
		ShaderManager& shaderManager,
		MaterialManager& materialManager,
		rMaterialManager& rmaterialManager,
		Camera& camera
	)
	{
		for (Entity entity : sceneRegistry.GetAll())
		{
			auto meshIt = meshManager.components.find(entity);
			if (meshIt == meshManager.components.end())
				continue;
			const MeshComponent& meshComp = meshIt->second;

			ShaderComponent* shaderComp = shaderManager.GetComponent(entity);
			MaterialComponent* materialComp = materialManager.GetComponent(entity);
			TransformComponent* transformComp = transformManager.GetComponent(entity);
			rMaterialComponent* rmaterialComp = rmaterialManager.GetComponent(entity);

			if (!shaderComp || !shaderComp->shader) continue;

			Shader* shader = shaderComp->shader;
			shader->use();

			glm::mat4 model = glm::mat4(1.0f);

			if (transformComp)
			{
				model = glm::translate(model, transformComp->position);
				model = glm::rotate(model, transformComp->rotation.x, glm::vec3(1, 0, 0));
				model = glm::rotate(model, transformComp->rotation.y, glm::vec3(0, 1, 0));
				model = glm::rotate(model, transformComp->rotation.z, glm::vec3(0, 0, 1));
				model = glm::scale(model, transformComp->scale);
			}

			shader->setMat4("model", model);
			glm::vec3 cameraPos(5.0f, 2.5f, 5.0f);
			glm::vec3 target(0.0f, 0.0f, 0.0f);
			glm::vec3 up(0.0f, 1.0f, 0.0f);
			glm::mat4 view = glm::lookAt(cameraPos, target, up);
			shader->setMat4("view", view);
			shader->setMat4("projection", glm::perspective(glm::radians(45.0f), (float)1600 / (float)1200, 0.1f, 10.0f));

			if (rmaterialComp)
			{
				rmaterialComp->material.ApplyShaderUniforms(*shader);
			}

			//if (materialComp)
			//{
			//	for (const auto& pair : materialComp->parameters)
			//	{
			//		std::string uniformName = pair.first;
			//		UniformValue uniformValue = pair.second;

			//		switch (uniformValue.type)
			//		{
			//			case UniformValue::Type::Bool:
			//				shader->setBool(uniformName, uniformValue.boolValue);
			//				break;
			//			case UniformValue::Type::Int:
			//				shader->setInt(uniformName, uniformValue.intValue);
			//				break;
			//			case UniformValue::Type::Float:
			//				shader->setFloat(uniformName, uniformValue.floatValue);
			//				break;
			//			case UniformValue::Type::Vec2:
			//				shader->setVec2(uniformName, uniformValue.vec2Value);
			//				break;
			//			case UniformValue::Type::Vec3:
			//				shader->setVec3(uniformName, uniformValue.vec3Value);
			//				break;
			//			case UniformValue::Type::Vec4:
			//				shader->setVec4(uniformName, uniformValue.vec4Value);
			//				break;
			//			case UniformValue::Type::Mat4:
			//				shader->setMat4(uniformName, uniformValue.mat4Value);
			//				break;
			//		}
			//	}
			//}

			if (meshComp.mesh)
				meshComp.mesh->Draw(*shader, true);

		}
		
	}
};