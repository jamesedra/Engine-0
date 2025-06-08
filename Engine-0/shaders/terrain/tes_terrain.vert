#version 410 core

layout (location = 0) in vec2 aGridPos;
layout(location = 1) in vec2 aTexCoords;

out VS_OUT {vec2 GridPos, TexCoord;} v_out;

void main() {
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0); // placeholder, set in eval shader
	v_out.GridPos = aGridPos;
	v_out.TexCoord = aTexCoords;
}