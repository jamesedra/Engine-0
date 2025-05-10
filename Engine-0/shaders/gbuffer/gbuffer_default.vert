#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBNMatrix;

// view space outs
out vec3 FragPosVS;
out vec3 NormalVS;
out mat3 TBNMatrixVS;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	FragPos = vec3(model * vec4(aPos, 1.0));
	FragPosVS = vec3(view * model * vec4(aPos,1));

	Normal = mat3(transpose(inverse(model))) * aNormal;
	
	TexCoords = aTexCoords;

	// TBN world matrix
	vec3 Tw = normalize(vec3(model * vec4(aTangent, 0.0)));
	vec3 Nw = normalize(vec3(model * vec4(aNormal, 0.0)));
	Tw = normalize(Tw - dot(Tw,Nw) * Nw);
	vec3 Bw = cross(Nw, Tw);
	TBNMatrix = (mat3(Tw, Bw, Nw));

	// TBN view matrix
	mat3 viewRot = mat3(view);
	vec3 Tv = normalize(viewRot * Tw);
	vec3 Nv = normalize(viewRot * Nw);
	vec3 Bv = cross(Nv, Tv);
	TBNMatrixVS = mat3(Tv, Bv, Nv);
	NormalVS  = Nv;

	gl_Position = projection * view * vec4(FragPos, 1.0);
}