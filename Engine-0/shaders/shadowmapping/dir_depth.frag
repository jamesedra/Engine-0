#version 450 core
layout (location = 0) out vec2 moments;

void main() {
	float d = gl_FragCoord.z;
	float dx = dFdx(d);
	float dy = dFdy(d);

	float sigma = 0.5;
	float var = sigma * sigma * (dx * dx + dy * dy);
	float bias = 0.0005;
	float db = d + bias;

	vec2 m;
	m.x = db;
	m.y = db * db + var;
	moments = m;

//	float d = gl_FragCoord.z;
//	moments = vec2(d, d*d);
}