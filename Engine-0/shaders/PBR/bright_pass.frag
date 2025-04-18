#version 330 core

out vec4 FragColor;

uniform sampler2D hdrScene;
uniform float threshold = 1.0;

in vec2 TexCoords;

vec3 luminance = vec3(0.2126,0.7152,0.0722);

void main()
{
	vec3 color = texture(hdrScene, TexCoords).rgb;
	float l = dot(color, luminance);

	FragColor = (l > threshold) ? vec4(color, 1.0) : vec4(0.0);
}