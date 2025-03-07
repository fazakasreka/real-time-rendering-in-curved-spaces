#version 330
precision highp float;

struct Light {
    vec3 La, Le;
    vec4 wLightPos;
};

struct Material {
    vec3 kd, ks, ka;
    float shininess, emission;
};

uniform Material material;
uniform Light[8] lights;    // light sources 
uniform int   nLights;
uniform sampler2D diffuseTexture;

const float EUC = 0.0;
const float SPH = 1.0;
const float HYP = -1.0;
uniform float curvature;

in  vec4 wNormal;       // interpolated world sp normal
in  vec4 wView;         // interpolated world sp view
in  vec4 wLight[8];     // interpolated world sp illum dir
in  vec2 texcoord;
out vec4 fragmentColor; // output goes to frame buffer

float dotGeom(vec4 u, vec4 v) {
    float LorentzSign = curvature == HYP ? -1.0 : 1.0;
    return u.x * v.x + u.y * v.y + u.z * v.z + LorentzSign * u.w * v.w;
}

void main() {
    vec4 N = normalize(wNormal);
    vec4 V = normalize(wView); 
    vec3 texColor = texture(diffuseTexture, texcoord).rgb;
    vec3 ka = material.ka * texColor;
    vec3 kd = material.kd * texColor;

    vec3 radiance = texColor * material.emission;
    for(int i = 0; i < nLights; i++) {
        vec4 L = normalize(wLight[i]);
        vec4 H = normalize(L + V);
        float cost = max(dotGeom(N, L), 0), cosd = max(dotGeom(N, H), 0);
        // kd and ka are modulated by the texture
        radiance += ka * lights[i].La + (kd * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
    }
    fragmentColor = vec4(radiance, 1);
}