#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gMetallicAO;
uniform sampler2D sceneDepth;
uniform sampler2D sceneHDR;		// pure non-linearized PBR pass
uniform sampler2D sceneColor;	// tonemapped, LDR
uniform sampler2D brightPass;
uniform sampler2D bloomPass;	// blurred brightPass

void main() {
	vec3 color = texture(sceneColor, TexCoords).rgb;
	FragColor = vec4(color, 1.0);
}