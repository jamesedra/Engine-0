#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

struct Material {
	bool useDiffuseTexture;
	bool useSpecularTexture;

	vec3 diffuse;
	float specular;

	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};
uniform Material material;

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);

	vec3 diffuse = material.useDiffuseTexture ? texture(material.texture_diffuse1, TexCoords).rgb : material.diffuse;
	float spec = material.useSpecularTexture ? texture(material.texture_specular1, TexCoords).r : material.specular;

	gAlbedoSpec.rgb = diffuse;
	gAlbedoSpec.a = spec;
}