#version 460 core

layout (vertices = 4) out;

in vec2 inTexCoords[];
out vec2 TexCoords[];

void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	TexCoords[gl_InvocationID] = inTexCoords[gl_InvocationID];

	if (gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = 16;
		gl_TessLevelOuter[1] = 16;
		gl_TessLevelOuter[2] = 16;
		gl_TessLevelOuter[3] = 16;

		gl_TessLevelInner[0] = 16;
		gl_TessLevelInner[1] = 16;
	}
}
