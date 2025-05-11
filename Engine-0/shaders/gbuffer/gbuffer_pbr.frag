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
	bool useDiffuseValue;
	bool useRoughnessValue;
	bool useMetallicValue;

	vec3 diffuse;
	float roughness;
	float metallic;

	sampler2D texture_diffuse1;
	sampler2D texture_normal1;
	sampler2D texture_specular1;
	sampler2D texture_ao1;
	sampler2D texture_roughness1;
	sampler2D texture_metallic1;
};
uniform Material material;

void main() {
	gPosition = FragPos;
	gPositionVS = FragPosVS;

	vec3 normal = texture(material.texture_normal1, TexCoords).xyz * 2.0 - 1.0;
	gNormal = normalize(TBNMatrix * normal);
	gNormalVS = normalize(TBNMatrixVS * normal);

	vec3 diffuse = material.useDiffuseValue ? material.diffuse : texture(material.texture_diffuse1, TexCoords).rgb;
	float roughness = material.useRoughnessValue ? material.roughness : texture(material.texture_roughness1, TexCoords).r;
	float metallic = material.useMetallicValue ? material.metallic : texture(material.texture_metallic1, TexCoords).r;
	float ao = texture(material.texture_ao1, TexCoords).r;

	gAlbedoRoughness = vec4(diffuse, roughness);
	gMetallicAO = vec2(metallic, ao);
}