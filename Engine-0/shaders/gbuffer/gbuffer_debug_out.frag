#version 330 core

layout (location = 0) out vec4 debugPosition;
layout (location = 1) out vec4 debugNormal;
layout (location = 2) out vec4 debugAlbedo;
layout (location = 3) out vec4 debugMetallic;
layout (location = 4) out vec4 debugRoughness;
layout (location = 5) out vec4 debugAO;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gMetallicAO;

void main() {
	debugPosition = vec4(vec3(texture(gPosition, TexCoords)), 1.0);
	debugNormal = vec4(vec3(texture(gNormal, TexCoords)), 1.0);
	debugAlbedo = vec4(vec3(texture(gAlbedoRoughness, TexCoords)).rgb, 1.0);
	debugMetallic = vec4(vec3(texture(gMetallicAO, TexCoords).r), 1.0);
	debugRoughness = vec4(vec3(texture(gAlbedoRoughness, TexCoords).a), 1.0);
	debugAO = vec4(vec3(texture(gMetallicAO, TexCoords).g), 1.0);
}