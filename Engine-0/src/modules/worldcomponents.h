#pragma once
#include "../common.h"
#include "shader.h"
#include <unordered_map>
#include "mesh.h"

// NOTE: This is a temporary header file used to store all needed components in one place first.
// As of the moment, this will mostly prioritize components that will be used for the render system I am building. Which is basically what OpenGL will render in the viewport window.


// Object Structs (Note that some components (such as ShaderComponent do not need one)
// mesh struct (different from the model class we have. May be changed and only used for testing)
struct TestMesh
{
    unsigned int VAO;
    unsigned int indexCount = 0;
    unsigned int vertexCount = 0;

    void Draw() const
    {
        glBindVertexArray(VAO);
        if (indexCount > 0)
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};

struct UniformValue
{
    enum class Type { Bool, Int, Float, Vec2, Vec3, Vec4, Mat4 } type;
    union
    {
        bool boolValue;
        int intValue;
        float floatValue;
        glm::vec2 vec2Value;
        glm::vec3 vec3Value;
        glm::vec4 vec4Value;
        glm::mat4 mat4Value;
    };

    UniformValue() : type(Type::Bool), boolValue(false) {}

    UniformValue(bool value) : type(Type::Bool), boolValue(value) {}
    UniformValue(int value) : type(Type::Int), intValue(value) {}
    UniformValue(float value) : type(Type::Float), floatValue(value) {}
    UniformValue(glm::vec2 value) : type(Type::Vec2), vec2Value(value) {}
    UniformValue(glm::vec3 value) : type(Type::Vec3), vec3Value(value) {}
    UniformValue(glm::vec4 value) : type(Type::Vec4), vec4Value(value) {}
    UniformValue(glm::mat4 value) : type(Type::Mat4), mat4Value(value) {}
};


// Component structs
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

// soon to be deprecated
struct TestMeshComponent
{
    TestMesh* mesh;
};

struct MeshComponent
{
    Mesh* mesh;
};

struct ShaderComponent
{
    Shader* shader;
};

// Holds instances of the uniforms from the shader
struct MaterialComponent
{
    std::unordered_map<std::string, UniformValue> parameters;
};

