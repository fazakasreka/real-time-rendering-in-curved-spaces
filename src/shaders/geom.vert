#version 330
precision highp float;

struct Light {
    vec3 La, Le;
    vec4 wLightPos;
};

uniform mat4  ScaleMatrix;
uniform mat4  RotateMatrix;
uniform mat4  TranslateMatrix;
uniform mat4  VPMatrix;

uniform Light[8] lights;
uniform int   nLights;

uniform vec4  wEye;

uniform float LorentzSign;
layout(location = 0) in vec4  eucVtxPos;            // pos in modeling space
layout(location = 1) in vec4  eucVtxNorm;      	 // normal in modeling space
layout(location = 2) in vec2  vtxUV;

out vec4 wNormal;		    // normal in world space
out vec4 wView;             // view in world space
out vec4 wLight[8];		    // light dir in world space
out vec2 texcoord;

float dotGeom(vec4 u, vec4 v) {
    return u.x * v.x + u.y * v.y + u.z * v.z + LorentzSign * u.w * v.w;
}

vec4 direction(vec4 to, vec4 from) {
    if (LorentzSign > 0) {
        float cosd = dotGeom(from, to);
        float sind = sqrt(1 - cosd * cosd);
        return (to - from * cosd)/sind;
    }
    if (LorentzSign < 0) {
        float coshd = -dotGeom(from, to);
        float sinhd = sqrt(coshd * coshd - 1);
        return (to - from * coshd)/sinhd;
    }
    return normalize(to - from);
}

vec4 transformPointToCurrentSpace(vec4 eucPoint) {
    vec3 P = eucPoint.xyz;
    float distance = length(P);
    if (distance < 0.0001f) return eucPoint;
    if (LorentzSign > 0) return vec4(P/distance * sin(distance), cos(distance));
    if (LorentzSign < 0) return vec4(P/distance * sinh(distance), cosh(distance));
    return eucPoint;
}

vec4 transformVectorToCurrentSpace(vec4 vector, vec4 point) {
	if (LorentzSign == 0) return vector;

    float alph = LorentzSign;
	float x = point.x;
	float y = point.y;
	float z = point.z;
	float w = point.w;
	return vector * mat4(
		vec4(1 - (alph * (x*x / (1 + w))),	-(alph * (x*y / (1 + w))),		-(alph * (x*z / (1 + w))),	-alph * x),
		vec4(-(alph * (y*x / (1 + w))),		1 - (alph * (y*y / (1 + w))),	-(alph * (y*z / (1 + w))),	-alph * y),
		vec4(-(alph * (z*x / (1 + w))),		-(alph * (z*y / (1 + w))),		1- (alph * (z*z / (1 + w))),	-alph * z),
		vec4(x,								y,								z,							w));
}
void main() {
    vec4 wPos = transformPointToCurrentSpace(eucVtxPos * ScaleMatrix * RotateMatrix) * TranslateMatrix;
    gl_Position = wPos * VPMatrix;

    for(int i = 0; i < nLights; i++) 
        wLight[i] = direction(transformPointToCurrentSpace(lights[i].wLightPos), wPos);
    wView  = direction(transformPointToCurrentSpace(wEye), wPos);
    if (LorentzSign == 0) {
        wNormal = RotateMatrix * eucVtxNorm;
        wNormal.w = 0;
    } else {
        wNormal = transformVectorToCurrentSpace(RotateMatrix * eucVtxNorm, wPos);
    }
    
    texcoord = vtxUV;
}