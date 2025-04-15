#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gMetallicAO;

uniform vec3 viewPos;

// temp lighting values
uniform vec3 lightPos;
uniform vec3 lightColor;

vec3 Fresnel(float cosTheta, vec3 F0);
vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness);
float NormalDistribution(float nDotH, float roughness);
float GeometryEq(float dotProd, float roughness);

const float PI = 3.14159265359;

void main() {
	// deferred attachment unpacking
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 n = normalize(texture(gNormal, TexCoords).rgb);
	vec4 ar = texture(gAlbedoRoughness, TexCoords);
	vec3 albedo = ar.rgb;
	float roughness = ar.a;
	vec2 ma = texture(gMetallicAO, TexCoords).rg;
	float metallic = ma.r;
	float ao = ma.g;

	albedo = pow(albedo, vec3(2.2));	// linearize albedo
	roughness = max(roughness, 0.0001);

	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0); // outgoing radiance

	vec3 v = normalize(viewPos - fragPos);
	vec3 l = normalize(lightPos - fragPos);
	vec3 h = normalize(v + l);

	// Non-PBR, but lighting helper
	float distance = length(lightPos - fragPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = lightColor * attenuation;

	// Dot product setup
	float nDotL = max(dot(n, l), 0.0);
	float vDotH = max(dot(v, h), 0.0);
	float nDotH = max(dot(n, h), 0.0);
	float nDotV = max(dot(n, v), 0.0);

	// Specular BRDF
	vec3 F = Fresnel(vDotH, F0);
	float D = NormalDistribution(nDotH, roughness);
	float G = GeometryEq(nDotL, roughness) * GeometryEq(nDotV, roughness);

	vec3 SpecBRDF_nom = D * G * F;
	float SpecBRDF_denom = 4.0 * nDotV * nDotL;
	vec3 SpecBRDF = SpecBRDF_nom / max(SpecBRDF_denom, 0.001);

	// Diffuse BRDF
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;
		
	vec3 fLambert = albedo;
	vec3 DiffuseBRDF = kD * fLambert / PI;

	Lo += (DiffuseBRDF + SpecBRDF) * radiance * nDotL;

	// commented out code is for irradiance map
	//vec3 kS = FresnelRoughness(max(dot(n, v), 0.0), F0, roughness);
	//vec3 kD = 1.0 - kS;
	//vec3 ambient = kD * texture(irradianceMap, n).rgb * albedo * ao;
	vec3 ambient = vec3(0.03) * albedo * ao; // base ambient value
	vec3 color = ambient + Lo;

	// HDR and gamma corrections
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	FragColor = vec4(color, 1.0);
}

// uses Fresnel-Schlick approximation
vec3 Fresnel(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(max((1.0 - cosTheta), 0.0), 5.0);
}

// based on Sebastien Lagarde's implementation
vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// uses TrowBridge-Reitz GGX
float NormalDistribution(float nDotH, float roughness) {
    float a2 = roughness * roughness;
    float denom = (nDotH * nDotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * (denom * denom));
}

// uses Schlick-Beckman GGX
float GeometryEq(float dotProd, float roughness) {
	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	return dotProd / (dotProd * (1.0 - k) + k);
}