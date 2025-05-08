#version 450 core

#define MAX_LIGHTS 512
#define MAX_LIGHTS_PER_TILE 64

out vec4 FragColor;
in vec2 TexCoords;

struct Light {
	vec4 pos_radius;
	vec4 color_intensity;
};

layout(std430, binding = 0) readonly buffer LightBuf {
	Light lights[MAX_LIGHTS];
};

layout(std430, binding = 1) readonly buffer TileInfoBuf {
	uvec2 tileInfo[];
};

layout(std430, binding = 2) readonly buffer LightIndexBuf {
	uint lightIndices[];
};

uniform ivec2 screenSize;
uniform ivec2 tileCount;
uniform int tileSize;
uniform vec3 viewPos;

void main() {
	ivec2 pixel = ivec2(gl_FragCoord.xy);
    ivec2 tile = pixel / tileSize;
    int checker  = (tile.x + tile.y) & 1;          
    float gray   = float(checker);                 
    FragColor    = vec4(vec3(gray), 1.0);
}