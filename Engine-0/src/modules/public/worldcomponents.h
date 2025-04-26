#pragma once

#include <unordered_map>
#include "mesh.h"
#include "material.h"
#include "shader.h"
#include "shader_uniform.h"
#include "ibl_generator.h"

// NOTE: This is a temporary header file used to store all needed components in one place first.
// As of the moment, this will mostly prioritize components that will be used for the render system I am building. Which is basically what OpenGL will render in the viewport window.

// Components
struct TransformComponent
{
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles (radians)
    glm::vec3 scale;

    TransformComponent(const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl)
    {
    }
};

// Identifier component, to put a name to an entity
struct IDComponent
{
    std::string ID;

    IDComponent() : ID("") { }
    IDComponent(const std::string& ID) : ID(ID) { }
};

// shaderName is different from the IDComponent. This is for identifying which object they are using from a library. 
// Eg. shaderName can be found in the ShaderLibrary
struct ShaderComponent
{
    std::string shaderName;
    Shader* shader;
};

struct AssetComponent
{
    std::string assetName; // refers to the asset library 
};


struct MaterialsGroupComponent
{
    std::vector<MaterialsGroup> materialsGroup;
};

struct EnvironmentProbeComponent
{
    std::string eqrMapPath; // path to *.hdr
    bool buildMap = true;   // for rebuilding
    IBLMaps maps{};         // four IBL texture cubemaps

    glm::vec3 position;
    float radius = std::numeric_limits<float>::infinity();
};
