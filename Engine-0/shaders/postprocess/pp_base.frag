#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;

void main() {
	FragColor = vec4(texture(scene, TexCoords).rgb, 1.0);
}