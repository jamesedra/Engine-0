#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D tonemappedScene;
uniform sampler2D sceneDepth;
uniform samplerCube skybox;
uniform mat4 invProjection;
uniform mat4 invView;

// reconstruct a world‐space ray from screen UV
vec3 reconstructDir(vec2 uv) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 view = invProjection * ndc;
    view /= view.w;
    return normalize((invView * vec4(view.xyz, 0.0)).xyz);
}

void main() {
    float d = texture(sceneDepth, TexCoords).r;
    if (d > 0.999) {
        vec3 dir = reconstructDir(TexCoords);
        vec3 skyLinear = texture(skybox, dir).rgb;
        vec3 skyTM = skyLinear / (skyLinear + vec3(1.0));
        const float gamma = 2.2;
        skyTM = pow(skyTM, vec3(1.0/gamma));
        FragColor = vec4(skyTM, 1.0);
    } else {
        FragColor = texture(tonemappedScene, TexCoords);
    }
}