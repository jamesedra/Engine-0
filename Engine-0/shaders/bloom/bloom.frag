#version 330 core

out vec4 FragColor;

uniform sampler2D hdrScene;
uniform sampler2D blurBuffer;
uniform float exposure;

in vec2 TexCoords;

void main() {
	vec3 scene = texture(hdrScene,  TexCoords).rgb;
    vec3 glow  = texture(blurBuffer,  TexCoords).rgb;

    vec3 color = scene + glow * exposure;
    FragColor  = vec4(color, 1.0);
}