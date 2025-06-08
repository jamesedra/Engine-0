#version 450 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;

uniform sampler2D grassTex;
uniform sampler2D rockTex;

void main() {
	float up = Normal.y * 0.5 + 0.5;
	vec3 grass = texture(grassTex, TexCoords * 0.4).rgb;
	vec3 rock = texture(rockTex, TexCoords * 2.5).rgb;

	vec3 color = mix(rock, grass, max(up - 0.2, 0.0)) * max(up, 0.4);
	FragColor = vec4(color, 1.0);
}