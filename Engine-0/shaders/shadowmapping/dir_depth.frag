#version 450 core
layout (location = 0) out vec2 moments;

void main() {
	float d = gl_FragCoord.z;

	vec2 m;
	m.x = d;
	// float dx = dFdx(d);
	// float dy = dFdy(d);

	m.y = d * d ; //+ 0.25 * (dx * dx + dy * dy);

	moments = m;
}