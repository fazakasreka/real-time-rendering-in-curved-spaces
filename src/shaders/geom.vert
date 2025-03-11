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

const float EUC = 0.0;
const float SPH = 1.0;
const float HYP = -1.0;
uniform float curvature;

uniform Light[8] lights;
uniform int   nLights;
uniform vec4  wEye;

layout(location = 0) in vec4  eucVtxPos;            // pos in modeling space
layout(location = 1) in vec4  eucVtxNorm;      	 // normal in modeling space
layout(location = 2) in vec2  vtxUV;

out vec4 wNormal;		    // normal in world space
out vec4 wView;             // view in world space
out vec4 wLight[8];		    // light dir in world space
out vec2 texcoord;

float dotGeom(vec4 u, vec4 v) {
    float LorentzSign = curvature == HYP ? -1.0 : 1.0;
    return u.x * v.x + u.y * v.y + u.z * v.z + LorentzSign * u.w * v.w;
}

vec4 direction(vec4 to, vec4 from) {
    if(curvature == EUC) {
        return normalize(to - from);
    }
    if (curvature == SPH) {
        float cosd = dotGeom(from, to);
        float sind = sqrt(1 - cosd * cosd);
        return (to - from * cosd)/sind;
    }
    //HYP
    float coshd = -dotGeom(from, to);
    float sinhd = sqrt(coshd * coshd - 1);
    return (to - from * coshd)/sinhd;
    
}

vec4 transformPointToCurrentSpace(vec4 eucPoint) {
    if (curvature == EUC) {
        return eucPoint;
    }
    
    vec3 P = eucPoint.xyz;
    float dist = sqrt(dot(P,P)) + 0.000001;
    vec4 v = vec4(P/dist, 0);

    if (curvature == SPH) {
        return vec4(0,0,0,1) * cos(dist) + v * sin(dist);
    }
    //HYP
    return vec4(0,0,0,1) * cosh(dist) + v * sinh(dist);
}
    

vec4 transformVectorToCurrentSpace(vec4 vector, vec4 point) {
	if (curvature == EUC) return vector;
    float alpha = curvature;
	float x = point.x;
	float y = point.y;
	float z = point.z;
	float w = point.w;
    //vector * TransformMatrix(point)
	return vector * mat4(
		vec4(1 - (alpha * (x*x / (1 + w))),	-(alpha * (x*y / (1 + w))),		-(alpha * (x*z / (1 + w))),	-alpha * x),
		vec4(-(alpha * (y*x / (1 + w))),		1 - (alpha * (y*y / (1 + w))),	-(alpha * (y*z / (1 + w))),	-alpha * y),
		vec4(-(alpha * (z*x / (1 + w))),		-(alpha * (z*y / (1 + w))),		1- (alpha * (z*z / (1 + w))),	-alpha * z),
		vec4(x,								y,								z,							w));
}
void main() {
    vec4 wPos = transformPointToCurrentSpace(
        eucVtxPos * ScaleMatrix * RotateMatrix
    ) * TranslateMatrix;
    gl_Position = wPos * VPMatrix;

    for(int i = 0; i < nLights; i++) {
        wLight[i] = direction(transformPointToCurrentSpace(lights[i].wLightPos), wPos);
    }
    
    wView  = direction(transformPointToCurrentSpace(wEye), wPos);

    wNormal = transformVectorToCurrentSpace(
        eucVtxNorm * transpose(inverse(ScaleMatrix * RotateMatrix)),
        wPos
    );

    texcoord = vtxUV;
}