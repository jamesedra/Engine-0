#pragma once
#include "camera.h"
#include "component_manager.h"
#include "shader.h"
#include "shader_storage_buffer.h"

static constexpr int MAX_LIGHTS = 1600;
static constexpr int MAX_LIGHTS_PER_TILE = 256;

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
    LightSystem(
        int screenWidth = 1600,
        int screenHeight = 1200,
        int tileSize = 16
    )
    {
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        this->tileSize = tileSize;
        this->numTilesX = (screenWidth + tileSize - 1) / tileSize;
        this->numTilesY = (screenHeight + tileSize - 1) / tileSize;
        this->tileCount = numTilesX * numTilesY;

        lightCompShader = Shader("shaders/lighting/lighting_tiled.comp");

        lightSSBO = ShaderStorageBuffer(0, 1, sizeof(GPULight) * MAX_LIGHTS);
        tileInfoSSBO = ShaderStorageBuffer(1, 1, sizeof(glm::uvec2) * tileCount);
        lightIndexSSBO = ShaderStorageBuffer(2, 1, sizeof(GLuint) * tileCount * MAX_LIGHTS_PER_TILE);
    }

    void TileLighting(
        SceneEntityRegistry& sceneRegistry,
        LightManager& lightManager,
        TransformManager& transformManager,
        Camera& camera)
    {
        std::vector<GPULight> lights;
        lights.reserve(MAX_LIGHTS);

        for (Entity entity : sceneRegistry.GetAll())
        {
            PointLightComponent* lightComp = lightManager.GetPointLightComponent(entity);
            TransformComponent* transformComp = transformManager.GetComponent(entity);
            if (!lightComp || !lightComp->enabled || !transformComp) continue;

            GPULight light{ 
                glm::vec4(transformComp->position, lightComp->radius),
                glm::vec4(lightComp->color, lightComp->intensity) 
            };

            lights.push_back(light);
            if (lights.size() >= MAX_LIGHTS) break;
        }
        int lightCount = (int)lights.size();
        lightSSBO.setData(0, sizeof(GPULight) * lightCount, lights.data());

        std::vector<glm::uvec2> tileData(tileCount);
        for (int t = 0; t < tileCount; ++t)
        {
            tileData[t].x = t * MAX_LIGHTS_PER_TILE;
            tileData[t].y = 0u;
        }
        tileInfoSSBO.setData(0, sizeof(glm::uvec2) * tileCount, tileData.data());

        lightCompShader.use();
        lightCompShader.setMat4("view", camera.getViewMatrix());
        lightCompShader.setMat4("projection", camera.getProjectionMatrix(screenWidth, screenHeight, 0.1f, 2500.0f));
        lightCompShader.setIVec2("screenSize", screenWidth, screenHeight);
        lightCompShader.setIVec2("tileCount", numTilesX, numTilesY);
        lightCompShader.setInt("tileSize", tileSize);
        lightCompShader.setInt("lightCount", lightCount);

        glDispatchCompute(numTilesX, numTilesY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void BindForShading() const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO.SSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tileInfoSSBO.SSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightIndexSSBO.SSBO);
    }

    void ConfigurePBRUniforms(
        Shader& pbrShader, 
        SceneEntityRegistry& sceneRegistry, 
        LightManager& lightManager,
        TransformManager& transformManager)
    {
        pbrShader.use();
        // BindForShading();
        pbrShader.setInt("tileSize", tileSize);
        pbrShader.setIVec2("screenSize", screenWidth, screenHeight);
        pbrShader.setIVec2("tileCount", numTilesX, numTilesY);

        auto dirLightCompEntity = lightManager.GetAnyDirectionalLight();
        if (!dirLightCompEntity) return;

        Entity e = dirLightCompEntity->first;

        // return if not part of the scene registry
        if (!sceneRegistry.Contains(e)) return;

        DirectionalLightComponent* dirLightComp = dirLightCompEntity->second;
        TransformComponent* transformComp = transformManager.GetComponent(e);

        pbrShader.setVec4("dirLight.pos_radius", glm::vec4(transformComp->rotation, 0.0f));
        pbrShader.setVec4("dirLight.color_intensity", glm::vec4(dirLightComp->color, dirLightComp->intensity));
    }
};
