#pragma once
#include "camera.h"
#include "component_manager.h"
#include "shader.h"
#include "shader_storage_buffer.h"

static constexpr int MAX_LIGHTS = 128;
static constexpr int MAX_LIGHTS_PER_TILE = 64;

// mirror struct from comp shader
struct GPULight
{
    glm::vec4 pos_radius;
    glm::vec4 color_intensity;
};

class LightSystem
{
private:
    Shader lightCompShader;
    ShaderStorageBuffer lightSSBO;
    ShaderStorageBuffer lightIndexSSBO;
    ShaderStorageBuffer tileInfoSSBO;
    int screenWidth, screenHeight;
    int tileSize, tileCount;
    int numTilesX,numTilesY;

public:
    void Initialize(int screenWidth = 1600, int screenHeight = 1200, int tileSize = 16)
    {
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        this->tileSize = tileSize;
        this->numTilesX = (screenWidth + tileSize - 1) / tileSize;
        this->numTilesY = (screenHeight + tileSize - 1) / tileSize;
        this->tileCount = numTilesX * numTilesY;

        lightCompShader = Shader("shaders/lighting/lighting_tiled.comp");

        lightSSBO = ShaderStorageBuffer(0, 1, sizeof(GPULight) * MAX_LIGHTS);
        tileInfoSSBO = ShaderStorageBuffer(1, 1, sizeof(glm::uvec2) * numTilesX * numTilesY);
        lightIndexSSBO = ShaderStorageBuffer(2, 1, sizeof(GLuint) * numTilesX * numTilesY * MAX_LIGHTS_PER_TILE);
    }

    void TileLighting(
        SceneEntityRegistry& sceneRegistry,
        LightManager& lightManager,
        TransformManager& transformManager,
        Shader& lightCompShader,
        Camera& camera)
    {
        std::vector<GPULight> lights;
        std::vector<glm::uvec2> tileDefault(numTilesX * numTilesY, glm::uvec2(0, 0));
        
        lights.reserve(MAX_LIGHTS);

        for (Entity entity : sceneRegistry.GetAll())
        {
            LightComponent* lightComp = lightManager.GetComponent(entity);
            TransformComponent* transformComp = transformManager.GetComponent(entity);
            if (!lightComp || !lightComp->enabled || !transformComp) continue;

            GPULight light{ 
                glm::vec4(transformComp->position, lightComp->radius),
                glm::vec4(lightComp->color, lightComp->intensity) 
            };

            lights.push_back(light);
            if ((int)lights.size() >= MAX_LIGHTS) break;
        }
        int lightCount = (int)lights.size();
        
        lightSSBO.setData(0, sizeof(GPULight) * lightCount, lights.data());
        tileInfoSSBO.setData(0, sizeof(glm::uvec2) * tileDefault.size(), tileDefault.data());

        lightCompShader.use();
        lightCompShader.setMat4("view", camera.getViewMatrix());
        lightCompShader.setMat4("projection", camera.getProjectionMatrix(screenWidth, screenHeight, 0.1f, 1000.0f));
        lightCompShader.setIVec2("screenSize", screenWidth, screenHeight);
        lightCompShader.setIVec2("tileCount", numTilesX, numTilesY);
        lightCompShader.setInt("tileSize", tileSize);

        glDispatchCompute(numTilesX, numTilesY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void BindForShading() const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO.SSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tileInfoSSBO.SSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightIndexSSBO.SSBO);
    }
};
