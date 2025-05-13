#version 450 core

#define MAX_LIGHTS 1600
#define MAX_LIGHTS_PER_TILE 64
#define MAX_PROBES 4

out vec4 FragColor;
in vec2 TexCoords;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gMetallicAO;
uniform sampler2D gPositionVS;

// Variance shadow mapping
uniform sampler2D dirVSM;
uniform mat4 lightSpaceMatrix;

// SSAO pass
uniform sampler2D ssaoLUT;

// IBL
uniform int probeCount;
uniform samplerCube irradianceMap[MAX_PROBES];
uniform samplerCube prefilterMap[MAX_PROBES];
uniform sampler2D brdfLUT[MAX_PROBES];
uniform vec4 probeData[MAX_PROBES]; // position = xyz, radius = w

// Lighting
struct Light {
	vec4 pos_radius;
	vec4 color_intensity;
};

// Directional light
uniform Light dirLight; // pos_radius only stores direction

// Point Light Tile Buffers
layout(std430, binding = 0) readonly buffer LightBuf {
	Light lights[MAX_LIGHTS];
};

layout(std430, binding = 1) readonly buffer TileInfoBuf {
	uvec2 tileInfo[];
};

layout(std430, binding = 2) readonly buffer LightIndexBuf {
	uint lightIndices[];
};

// Tile Data
uniform ivec2 screenSize;
uniform ivec2 tileCount;
uniform int tileSize;

uniform vec3 viewPos;

vec3 Fresnel(float cosTheta, vec3 F0);
vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness);
float NormalDistribution(float nDotH, float roughness);
float GeometryEq(float dotProd, float roughness);
ivec2 FindClosestProbes(vec3 fragPos);
float DirShadowContribution(vec2 LightTexCoord, float DistToLight);

const float PI = 3.14159265359;
const float g_MinVariance = 1e-4;

void main() {
	// deferred attachment unpacking
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 n = normalize(texture(gNormal, TexCoords).rgb);

	vec4 ar = texture(gAlbedoRoughness, TexCoords);
	vec3 albedo = ar.rgb;
	float roughness = ar.a;
	roughness = max(roughness, 0.0001);

	vec2 ma = texture(gMetallicAO, TexCoords).rg;
	float metallic = ma.r;
	float ao = texture(ssaoLUT, TexCoords).r * ma.g;
	ao = max(ao, 0.1);

	vec3 v = normalize(viewPos - fragPos);
	float nDotV = max(dot(n, v), 0.0);

	ivec2 pixel = ivec2(gl_FragCoord.xy);
    ivec2 tile = pixel / tileSize;
    int tileID = tile.y * tileCount.x + tile.x;

	uint offset = tileInfo[tileID].x;
	uint count  = tileInfo[tileID].y;

	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 Lo = vec3(0.0); // outgoing radiance

	// Point Lights
	for (uint i = 0u; i < count; i++) {
		uint lightID = lightIndices[offset + i];
		Light light = lights[lightID];

		// light unpacking
		vec3 lightPos = light.pos_radius.rgb;
		float lightRadius = light.pos_radius.a;
		vec3 lightColor = light.color_intensity.rgb;
		float lightIntensity = light.color_intensity.a;

		vec3 l = normalize(lightPos - fragPos);
		vec3 h = normalize(v + l);

		// lighting helper, tentative
		float distance = length(lightPos - fragPos);
		if (distance > lightRadius) continue;

		float radius2 = lightRadius * lightRadius;
		float dist2   = dot(lightPos - fragPos, lightPos - fragPos);
		// attenuation testing
		// float attenuation = dist2 < radius2 ? 1.0 : clamp(radius2 / dist2, 0.0, 1.0);
		// float attenuation = clamp(radius2 / (distance * distance), 0.0, 1.0);
		float attenuation = 1.0 - (distance / lightRadius);

		// Dot product setup
		float nDotL = max(dot(n, l), 0.0);
		float vDotH = max(dot(v, h), 0.0);
		float nDotH = max(dot(n, h), 0.0);
		
		if (nDotL > 0.0) {
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

			vec3 radiance = lightColor * attenuation * lightIntensity;
			Lo += (DiffuseBRDF + SpecBRDF) * radiance * nDotL;
		}
	}

	// directional light
	vec3 Ld = normalize(-dirLight.pos_radius.xyz);
	vec3 h = normalize(v + Ld);
	float nDotL = max(dot(n, Ld), 0.0);
	float vDotH = max(dot(v, h), 0.0);
	float nDotH = max(dot(n, h), 0.0);
	if (nDotL > 0.0) {
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

		vec3 radiance = dirLight.color_intensity.rgb * dirLight.color_intensity.a;

		Lo += (DiffuseBRDF + SpecBRDF) * radiance * nDotL;
	}

	// IBL
	vec3 R = reflect(-v, n);
	const float MAX_REFLECTION_LOD = 4.0;

	vec3 F_ibl = FresnelRoughness(nDotV, F0, roughness);
	vec3 kS = F_ibl;
	vec3 kD = 1.0 - kS;
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
		// get maps from probes[0], skybox
		vec3 irradiance = texture(irradianceMap[0], n).rgb;
		vec3 diffuseIBL = irradiance * albedo;
		vec3 prefilteredColor = textureLod(prefilterMap[0], R, roughness * MAX_REFLECTION_LOD).rgb;
		vec2 envBRDF = texture(brdfLUT[0], vec2(nDotV, roughness)).rg;
		vec3 specularIBL = prefilteredColor * (F_ibl * envBRDF.x + envBRDF.y);

		ambient = kD * diffuseIBL * ao + specularIBL;
	}
	else if (probeCount >= 2) {
		// get two nearest probes and blend
		ivec2 probeIdx = FindClosestProbes(fragPos);
		int i0 = probeIdx.x;
		int i1 = probeIdx.y;

		vec3 p0 = probeData[i0].xyz;
		vec3 p1 = probeData[i1].xyz;
		float r0 = probeData[i0].w;
		float r1 = probeData[i1].w;

		float d0 = length(fragPos - p0);
		float d1 = length(fragPos - p1);

		// cubic smoothsetp (1-d/r)^3
		float w0 = clamp(1.0 - d0 / r0, 0.0, 1.0);
		float w1 = clamp(1.0 - d1 / r1, 0.0, 1.0);
		w0 *= w0 * w0;
		w1 *= w1 * w1;

		float wSky = max(1.0 - (w0 + w1), 0.0);

		vec3 irrSky = texture(irradianceMap[0], n).rgb;
		vec3 preSky = textureLod(prefilterMap[0], R, roughness * MAX_REFLECTION_LOD).rgb;
		vec2 envSky = texture(brdfLUT[0], vec2(nDotV, roughness)).rg;

		vec3 irr0 = texture(irradianceMap[i0], n).rgb;
		vec3 irr1 = texture(irradianceMap[i1], n).rgb;
		irradiance = irr0 * w0 + irr1 * w1 + irrSky * wSky;

		vec3 pre0 = textureLod(prefilterMap[i0], R, roughness * MAX_REFLECTION_LOD).rgb;
		vec3 pre1 = textureLod(prefilterMap[i1], R, roughness * MAX_REFLECTION_LOD).rgb;
		prefilteredColor = pre0 * w0 + pre1 * w1 + preSky * wSky;

		vec2 env0 = texture(brdfLUT[i0], vec2(nDotV, roughness)).rg;
		vec2 env1 = texture(brdfLUT[i1], vec2(nDotV, roughness)).rg;
		envBRDF = env0 * w0 + env1 * w1 + envSky * wSky;

		vec3 diffuseIBL = irradiance * albedo;
		vec3 specularIBL = prefilteredColor * (F_ibl * envBRDF.x + envBRDF.y);
		ambient = (kD * diffuseIBL * ao) + specularIBL;
	}

	// Shadow mapping
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
	vec3 ndc = fragPosLightSpace.xyz / fragPosLightSpace.w;
	vec3 lightTexCoord = ndc * 0.5 + 0.5;

	vec2 moments = texture(dirVSM, lightTexCoord.xy).rg;
	FragColor = vec4(vec3(moments.x), 1.0);

	vec3 color = ambient + Lo;
	// FragColor = vec4(color, 1.0);
	
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

float ChebyshevUpperBound(vec2 moments, float t) {
	float  p = (t <= moments.x) ? 1.0 : 0.0;
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, g_MinVariance);
	float d = t - moments.x;
	float p_max = variance / (variance + d * d);
	return max(p, p_max);
}

float DirShadowContribution(vec2 LightTexCoord, float DistToLight) {
	vec2 moments = texture(dirVSM, LightTexCoord).xy;
	return ChebyshevUpperBound(moments, DistToLight);
}

ivec2 FindClosestProbes(vec3 fragPos) {
	int idx0 = -1, idx1 = -1;
	float d0 = 1e20, d1 = 1e20;

	// ignore index 0
	for (int i = 1; i < probeCount && i < MAX_PROBES; i++) {
		float di = length(fragPos - probeData[i].rgb);

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