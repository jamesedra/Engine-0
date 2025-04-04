#pragma once
#include "../common.h"
#include "shader.h"
#include "model.h"

class WorldComponent
{
public:
	virtual ~WorldComponent() = default;
};

struct Transform : public WorldComponent
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Transform(const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl)
    {
    }
};

class ModelRenderer : public WorldComponent
{
public:
    Model* model;
    Shader* shader;

    ModelRenderer(Model* model, Shader* shader)
        : model(model), shader(shader)
    {
    }
};