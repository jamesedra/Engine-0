#version 450 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;

uniform sampler2D terrainTex;

void main() {
	vec3 color = texture(terrainTex, TexCoords * 0.4).rgb * max((Normal.y * 0.5 + 0.5), 0.05);
	FragColor = vec4(1.0); // test
}