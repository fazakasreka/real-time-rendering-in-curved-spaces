#version 330
precision highp float;

struct Light {
    vec3 La, Le;
    vec4 wLightPos;
};

uniform mat4  RotateMatrix; // MVP, Model, Model-inverse
uniform vec4  eucTranslate; 
uniform Light[8] lights;    // light sources 
uniform int   nLights;
uniform vec4  wEye, ic, jc, kc;         // pos of eye
uniform float sFovX, sFovY, fp, bp;
uniform float wMirror;
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

vec4 portEucToGeom(vec4 eucPoint) {
    vec3 P = eucPoint.xyz;
    float distance = length(P);
    if (distance < 0.0001f) return eucPoint;
    if (LorentzSign > 0) return vec4(P/distance * sin(distance), cos(distance));
    if (LorentzSign < 0) return vec4(P/distance * sinh(distance), cosh(distance));
    return eucPoint;
}

mat4 TranslateMatrix(vec4 to) {
    if (LorentzSign != 0) {
        float denom = 1 + to.w;
        return transpose(mat4(1 - LorentzSign * to.x * to.x / denom,   - LorentzSign * to.x * to.y / denom,   - LorentzSign * to.x * to.z / denom, -LorentzSign * to.x,
                                - LorentzSign * to.y * to.x / denom, 1 - LorentzSign * to.y * to.y / denom,   - LorentzSign * to.y * to.z / denom, -LorentzSign * to.y,
                                - LorentzSign * to.z * to.x / denom,   - LorentzSign * to.z * to.y / denom, 1 - LorentzSign * to.z * to.z / denom, -LorentzSign * to.z,
                                                                to.x,                                  to.y,                   to.z,                 to.w));
    }
    return transpose(mat4(1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            to.x, to.y, to.z, 1));
} 

mat4 ViewMatrix(vec4 eye, vec4 icp, vec4 jcp, vec4 kcp) {
    if (LorentzSign != 0) {
        return transpose(mat4(icp.x, jcp.x, kcp.x, LorentzSign * eye.x,
                                icp.y, jcp.y, kcp.y, LorentzSign * eye.y,
                                icp.z, jcp.z, kcp.z, LorentzSign * eye.z,
                                LorentzSign * icp.w, LorentzSign * jcp.w, LorentzSign * kcp.w, eye.w));
    }

    return transpose(mat4(icp.x, jcp.x, kcp.x, 0,
                            icp.y, jcp.y, kcp.y, 0,
                            icp.z, jcp.z, kcp.z, 0,
                            -dotGeom(icp, eye), -dotGeom(jcp, eye), -dotGeom(kcp, eye), 1));
}

mat4 ProjectionMatrix( ) {
    if (LorentzSign > 0) {
        return transpose(mat4(sFovX, 0, 0, 0,
                                0, sFovY, 0, 0,
                                0, 0, 0, -1,
                                0, 0, -fp, 0));
    }
    else {
        return transpose(mat4(sFovX, 0, 0, 0,
                0, sFovY, 0, 0,
                0, 0, -(fp + bp) / (bp - fp), -1,
                0, 0, -2 * fp*bp / (bp - fp), 0));
    }
}

void main() {
    vec4 geomPoint = portEucToGeom(eucVtxPos);
    vec4 geomTranslate = portEucToGeom(eucTranslate);
    mat4 ModelMatrix = RotateMatrix * TranslateMatrix(geomTranslate);
    vec4 geomEye = portEucToGeom(wEye);
    mat4 eyeTranslate = TranslateMatrix(geomEye);
    vec4 geomCi = ic * eyeTranslate, geomCj = jc * eyeTranslate, geomCk = kc * eyeTranslate; 
    mat4 MVP = ModelMatrix * ViewMatrix(geomEye, geomCi, geomCj, geomCk) * ProjectionMatrix();
    gl_Position = wMirror * geomPoint * MVP; // to NDC
    vec4 wPos = geomPoint * ModelMatrix;
    for(int i = 0; i < nLights; i++) 
        wLight[i] = direction(portEucToGeom(lights[i].wLightPos), wPos);
    wView  = direction(portEucToGeom(wEye), wPos);
    if (LorentzSign == 0) {
        wNormal = RotateMatrix * eucVtxNorm;
        wNormal.w = 0;
    } else {
        wNormal = eucVtxNorm * ModelMatrix;
    }
    texcoord = vtxUV;
}