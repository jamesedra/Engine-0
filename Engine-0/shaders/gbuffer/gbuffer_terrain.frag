#version 330 core

// world space outs
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoRoughness;
layout (location = 3) out vec2 gMetallicAO;

// view space outs
layout (location = 4) out vec3 gPositionVS;
layout (location = 5) out vec3 gNormalVS;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 FragPosVS;
in vec3 NormalVS;
in mat3 TBNMatrix;
in mat3 TBNMatrixVS;

struct Material {
	bool useDiffuseValue1;
	bool useDiffuseValue2;
	bool useRoughnessValue1;
	bool useRoughnessValue2;

	vec3 diffuse1;
	vec3 diffuse2;
	float roughness1;
	float roughness2;

	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;

	sampler2D texture_normal1;
	sampler2D texture_normal2;

	sampler2D texture_roughness1;
	sampler2D texture_roughness2;

	sampler2D texture_ao1;
	sampler2D texture_ao2;
};
uniform Material material;

void main() {
	gPosition = FragPos;
	gPositionVS = FragPosVS;

	float up = Normal.y * 0.5 + 0.5;

	// textures based on slope
	vec3 n1 = texture(material.texture_normal1, TexCoords).xyz * 2.0 - 1.0;
	vec3 n2 = texture(material.texture_normal2, TexCoords).xyz * 2.0 - 1.0;

	vec3 d1 = material.useDiffuseValue1 ? material.diffuse1 : texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 d2 = material.useDiffuseValue2 ? material.diffuse2 : texture(material.texture_diffuse2, TexCoords).rgb;

	float r1 = material.useRoughnessValue1 ? material.roughness1 : texture(material.texture_roughness1, TexCoords).r;
	float r2 = material.useRoughnessValue2 ? material.roughness2 : texture(material.texture_roughness2, TexCoords).r;

	float ao1 = texture(material.texture_ao1, TexCoords).r;
	float ao2 = texture(material.texture_ao2, TexCoords).r;

	float ao = mix(ao1, ao2, up);
	vec3 diffuse = mix(d1, d2, up);
	float roughness = mix(r1, r2, up);
	vec3 normal = mix(n1, n2, up);
	gNormal = normalize(TBNMatrix * normal);
	gNormalVS = normalize(TBNMatrixVS * normal);

	gAlbedoRoughness = vec4(diffuse, roughness);
	gMetallicAO = vec2(0.0, ao);
}