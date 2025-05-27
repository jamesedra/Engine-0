#version 460 core

layout (vertices = 4) out;

in VS_OUT {vec2 GridPos, TexCoord;} tcs_in[];
out TCS_OUT {vec2 GridPos, TexCoord;} tcs_out[];

uniform float tessFactor = 8.0;

void main() {
    tcs_out[gl_InvocationID].GridPos = tcs_in[gl_InvocationID].GridPos;
    tcs_out[gl_InvocationID].TexCoord = tcs_in[gl_InvocationID].TexCoord;

    if (gl_InvocationID == 0) {
        for (int i = 0; i < 4; ++i) gl_TessLevelOuter[i] = tessFactor;
        gl_TessLevelInner[0] = tessFactor;
    }
}
