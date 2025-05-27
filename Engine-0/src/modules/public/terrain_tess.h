#pragma once
#include "terrain.h"

class TessTerrain : public Terrain
{
public:
	void InitializePatches();
	void Render(Shader& shader, Camera& camera) override;
};