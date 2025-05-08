#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gMetallicAO;
uniform sampler2D sceneDepth;
uniform sampler2D sceneHDR;		    // pure non-linearized PBR pass
uniform sampler2D sceneColor;	    // tonemapped, LDR
uniform sampler2D brightPass;
uniform sampler2D bloomPass;	    // blurred brightPass
uniform sampler2D compositePass;    // final output

vec3 RGBtoHSV(vec3 c);
vec3 HSVtoRGB(vec3 c);
vec3 desaturation(vec3 color, float saturation);

const float gamma = 2.2;

void main() {
	vec3 pp0 = texture(sceneHDR, TexCoords).rgb;
    vec3 baseColor = max(texture(gAlbedoRoughness, TexCoords).rgb, 0.0001);
    vec3 hsv = RGBtoHSV(pp0 / baseColor);
    float v = hsv.z;
    float p_v = pow(2, round(log2(v)));
    vec3 cel = HSVtoRGB(vec3(hsv.xy, p_v));
    vec3 celColor = baseColor * cel;

    // gamma correction and hdr
    vec3 mapped = celColor / (celColor + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / gamma));

    float d = texture(sceneDepth, TexCoords).r;
    if (d > 0.999) FragColor = texture(compositePass, TexCoords);
    else FragColor = vec4(mapped, 1.0);
}

vec3 desaturation(vec3 color, float saturation) {
	float Y = dot(color, vec3(0.30, 0.59, 0.11));
	return mix(color, vec3(Y), saturation);
}

vec3 RGBtoHSV(vec3 c) {
    float maxc = max(c.r, max(c.g, c.b));
    float minc = min(c.r, min(c.g, c.b));
    float d    = maxc - minc;
    float h = 0.0;
    if (d > 0.0) {
      if (maxc == c.r) h = mod((c.g - c.b) / d, 6.0);
      else if (maxc == c.g) h = (c.b - c.r) / d + 2.0;
      else h = (c.r - c.g)/d + 4.0;
      h /= 6.0;
    }
    float s = maxc > 0.0 ? d / maxc : 0.0;
    return vec3(h, s, maxc);
}

vec3 HSVtoRGB(vec3 c) {
    float h = c.x * 6.0;
    float s = c.y, v = c.z;
    int   i = int(floor(h));
    float f = fract(h);
    float p = v * (1.0 - s);
    float q = v * (1.0 - s * f);
    float t = v * (1.0 - s * (1.0 - f));
    if (i == 0) return vec3(v, t, p);
    else if (i == 1) return vec3(q, v, p);
    else if (i == 2) return vec3(p, v, t);
    else if (i == 3) return vec3(p, q, v);
    else if (i == 4) return vec3(t, p, v);
    else return vec3(v, p, q);
}