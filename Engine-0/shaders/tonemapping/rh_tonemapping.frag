#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D hdrScene;
uniform float exposure;

void main() {
	const float gamma = 2.2;
	vec3 hdrColor = texture(hdrScene, TexCoords).rgb;

	// reinhard tonemapping
	vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
	// filmic tonemapping
	// mapped = vec3(1.0) - exp(-hdrColor * exposure);
	mapped = pow(mapped, vec3(1.0 / gamma));
	FragColor = vec4(mapped, 1.0);
}