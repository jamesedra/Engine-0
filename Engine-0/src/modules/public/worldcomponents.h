#pragma once

#include <unordered_map>
#include <variant>
#include "mesh.h"
#include "material.h"
#include "shader.h"
#include "shader_uniform.h"
#include "ibl_generator.h"
#include "terrain.h"

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
    IBLSettings settings;   // holds the resolution and eqr path
    IBLMaps maps{};         // four IBL texture cubemaps
    bool buildProbe = true;   // for re/building
    glm::vec3 position;
    float radius = std::numeric_limits<float>::infinity();
};

struct PointLightComponent
{
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
    float intensity = 0.0f;
    float radius = 0.0f;
    bool enabled = true;
};

struct DirectionalLightComponent
{
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
    float intensity = 0.0f;
    bool enabled = true;
};

// Terrain component and subcomponents
// since a Terrain's height can be any one of its generators, we will have
// subcomponents to make this work

struct FaultGenParams
{
    int iterations;
    float filter;
    int width = -1;
    int depth = -1;
};

struct MidpointGenParams
{
    float roughness;
    int size = -1;
};

struct HeightmapParams
{
    std::string filename;
};

using HeightGenParams = 
    std::variant<FaultGenParams, 
                 MidpointGenParams, 
                 HeightmapParams>;

struct HeightGenComponent
{
    HeightGenParams params;
    float heightScale = 1.0f;
    bool regenerate = true;
};

struct LandscapeComponent
{
    std::unique_ptr<Terrain> terrain;
};