#version 330 core
layout (location = 0) out vec2 moments;

void main() {
	float d = gl_FragCoord.z;
	moments = vec2(d, d*d);
}