#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D out_attachment;

void main() {
	vec3 color = texture(out_attachment, TexCoords).rgb;
	FragColor = vec4(color, 1.0);
}