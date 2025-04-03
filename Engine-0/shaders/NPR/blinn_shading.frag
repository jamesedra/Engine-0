#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

struct Light {
	vec3 Position;
	vec3 Color;
};

uniform Light dirLight;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;

void main() {
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
	float Specular = texture(gAlbedoSpec, TexCoords).a;

	vec3 lighting = Albedo * 0.3; // hard-coded ambient component
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 lightDir = normalize(dirLight.Position - FragPos);
	vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * dirLight.Color;
	lighting += diffuse;

	FragColor = vec4(lighting, 1.0);
}