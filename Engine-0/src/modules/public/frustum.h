#pragma once
#include "../../common.h"

struct Frustum
{
	glm::vec4 planes[6];

	/*
	 * constructor
	 * input: view projection matrix (proj * view)
	 */
	Frustum(const glm::mat4& vp)
	{
		glm::vec4 row0 = glm::vec4(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
		glm::vec4 row1 = glm::vec4(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
		glm::vec4 row2 = glm::vec4(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
		glm::vec4 row3 = glm::vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);

		planes[0] = row3 + row0; // left
		planes[1] = row3 - row0; // right
		planes[2] = row3 + row1; // bottom
		planes[3] = row3 - row1; // top
		planes[4] = row3 + row2; // near
		planes[5] = row3 - row2; // far

		for (int i = 0; i < 6; i++)
		{
			float len = glm::length(glm::vec3(planes[i]));
			planes[i] /= len;
		}
	}

	bool IsPatchSphereInFrustum(glm::vec3 center, float radius) const
	{
		for (int i = 0; i < 6; i++)
		{
			float dist = glm::dot(glm::vec3(planes[i]), center) + planes[i].w;
			if (dist < -radius) return false;
		}
		return true;
	}
};