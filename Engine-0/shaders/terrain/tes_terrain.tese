#version 460 core
layout (quads, fractional_even_spacing, ccw) in;

in TCS_OUT {vec2 GridPos, TexCoord;} tes_in[];
out TES_OUT {vec3 worldPos; vec3 normal; vec2 uv;} tes_out;

uniform sampler2D heightMap;
uniform float heightScale;
uniform vec2 terrainScale;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

vec2 bilerp(vec2 a, vec2 b, vec2 c, vec2 d)
{
    vec2 ab = mix(a,b, gl_TessCoord.x);
    vec2 cd = mix(c,d, gl_TessCoord.x);
    return mix(ab,cd, gl_TessCoord.y);
}

void main() {
    vec2 uv   = bilerp(tes_in[0].TexCoord, tes_in[1].TexCoord, tes_in[3].TexCoord, tes_in[2].TexCoord);
    vec2 grid = bilerp(tes_in[0].GridPos, tes_in[1].GridPos, tes_in[3].GridPos, tes_in[2].GridPos);

    float h = texture(heightMap, uv).r * heightScale;
    vec3 pos = vec3(grid.x * terrainScale.x, h, grid.y * terrainScale.y);

    float eps = 1.0 / textureSize(heightMap,0).x;
    float hL = texture(heightMap, uv + vec2(-eps,0)).r * heightScale;
    float hR = texture(heightMap, uv + vec2( eps,0)).r * heightScale;
    float hD = texture(heightMap, uv + vec2(0,-eps)).r * heightScale;
    float hU = texture(heightMap, uv + vec2(0, eps)).r * heightScale;
    vec3 n = normalize(vec3((hL-hR) * terrainScale.x, 2.0 * eps, (hD-hU) * terrainScale.y));

    tes_out.worldPos = pos;
    tes_out.normal = n;
    tes_out.uv = uv;

    gl_Position = projection * view * model * vec4(pos,1.0);
}