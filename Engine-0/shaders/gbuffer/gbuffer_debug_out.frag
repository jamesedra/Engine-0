#version 330 core

layout (location = 0) out vec4 debugPosition;
layout (location = 1) out vec4 debugNormal;
layout (location = 2) out vec4 debugAlbedo;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

void main() {
	debugPosition = vec4(vec3(texture(gPosition, TexCoords)), 1.0);
	debugNormal = vec4(vec3(texture(gNormal, TexCoords)), 1.0);
	debugAlbedo = vec4(vec3(texture(gAlbedoSpec, TexCoords)).rgb, 1.0);
}