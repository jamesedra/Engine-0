#version 450 core

#define MAX_PROBES 4
out vec4 FragColor;
in vec2 TexCoords;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gMetallicAO;

// IBL
uniform int probeCount;
uniform samplerCube irradianceMap[MAX_PROBES];
uniform samplerCube prefilterMap[MAX_PROBES];
uniform sampler2D brdfLUT[MAX_PROBES];
uniform vec3 probePosition[MAX_PROBES];

// temp lighting values
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 viewPos;

vec3 Fresnel(float cosTheta, vec3 F0);
vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness);
float NormalDistribution(float nDotH, float roughness);
float GeometryEq(float dotProd, float roughness);
ivec2 FindClosestProbes(vec3 fragPos);

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

	roughness = max(roughness, 0.0001);

	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0); // outgoing radiance

	vec3 v = normalize(viewPos - fragPos);
	vec3 l = normalize(lightPos - fragPos);
	vec3 h = normalize(v + l);

	// Non-PBR, but lighting helper // tentative for brightness check
	float intensity = 10.0;
	float radius = 2.5;
	float radius2 = radius * radius;
	float distance = length(lightPos - fragPos);
	float dist2   = dot(lightPos - fragPos, lightPos - fragPos);
	float attenuation = dist2 < radius2 ? 1.0 : clamp(radius2 / dist2, 0.0, 1.0);
	vec3 radiance = lightColor * attenuation * intensity;

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

	// IBL
	vec3 R = reflect(-v, n);
	const float MAX_REFLECTION_LOD = 4.0;

	vec3 F_ibl = FresnelRoughness(nDotV, F0, roughness);
	kS = F_ibl;
	kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance;
	vec3 prefilteredColor;
	vec2 envBRDF;
	vec3 ambient;

	// Get map data from probes
	if (probeCount == 0) {
		// constant ambient value
		ambient = vec3(0.03) * albedo * ao;
	} 
	else if (probeCount == 1) {
		// get maps from probes[0]
		vec3 irradiance = texture(irradianceMap[0], n).rgb;
		vec3 diffuseIBL = irradiance * albedo;
		vec3 prefilteredColor = textureLod(prefilterMap[0], R, roughness * MAX_REFLECTION_LOD).rgb;
		vec2 envBRDF = texture(brdfLUT[0], vec2(nDotV, roughness)).rg;
		vec3 specularIBL = prefilteredColor * (F_ibl * envBRDF.x + envBRDF.y);

		ambient = (kD * diffuseIBL + specularIBL) * ao;
	}
	else if (probeCount >= 2) {
		// get two nearest probes and blend
		ivec2 probeIdx = FindClosestProbes(fragPos);
		int i0 = probeIdx.x;
		int i1 = probeIdx.y;

		float d0 = length(fragPos - probePosition[i0]);
		float d1 = length(fragPos - probePosition[i1]);

		float w0 = d1 / (d0 + d1);
		float w1 = 1.0 - w0;

		vec3 irr0 = texture(irradianceMap[i0], n).rgb;
		vec3 irr1 = texture(irradianceMap[i1], n).rgb;
		irradiance = mix(irr0, irr1, w1);

		vec3 pre0 = textureLod(prefilterMap[i0], R, roughness * MAX_REFLECTION_LOD).rgb;
		vec3 pre1 = textureLod(prefilterMap[i1], R, roughness * MAX_REFLECTION_LOD).rgb;
		prefilteredColor = mix(pre0, pre1, w1);

		envBRDF = texture(brdfLUT[i0], vec2(nDotV, roughness)).rg;  

		vec3 diffuseIBL = irradiance * albedo;
		vec3 specularIBL = prefilteredColor * (F_ibl * envBRDF.x + envBRDF.y);
		ambient = (kD * diffuseIBL + specularIBL) * ao;
	}

	vec3 color = ambient + Lo;
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

ivec2 FindClosestProbes(vec3 fragPos) {
	int idx0 = -1, idx1 = -1;
	float d0 = 1e20, d1 = 1e20;

	for (int i = 0; i < probeCount && i < MAX_PROBES; i++) {
		float di = length(fragPos - probePosition[i]);

		if (di < d0) {
			d1 = d0;  
			idx1 = idx0;
			d0 = di;  
			idx0 = i;
		} else if (di < d1) {
			d1 = di;  
			idx1 = i;
		}
	}
	return ivec2(idx0, idx1);
}