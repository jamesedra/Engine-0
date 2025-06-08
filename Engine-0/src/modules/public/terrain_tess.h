#pragma once
#include "terrain.h"

class TessTerrain : public Terrain
{
public:
	void Initialize() override;
	void Render(Shader& shader, Camera& camera, glm::mat4& model) override;
};