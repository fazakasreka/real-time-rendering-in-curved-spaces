#version 330

struct Light {
    vec3 La, Le;
    vec4 wLightPos;
};

uniform mat4  ScaleMatrix;
uniform mat4  RotateMatrix;
uniform mat4  TranslateMatrix;
uniform mat4  VPMatrix;

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
    float LorentzSign = curvature < 0.0 ? -1.0 : 1.0;
    return u.x * v.x + u.y * v.y + u.z * v.z + LorentzSign * u.w * v.w;
}

vec4 direction(vec4 to, vec4 from) {
    if(curvature == 0.0) { //EUCLIDEAN
        return normalize(to - from);
    }
    if (curvature > 0.0) { //SPHERICAL
        float cosd = dotGeom(from, to);
        float sind = sqrt(1.0 - cosd * cosd);
        return (to - from * cosd)/sind;
    }
    if (curvature < 0.0) { //HYPERBOLIC
        float coshd = -dotGeom(from, to);
        float sinhd = sqrt(coshd * coshd - 1.0);
        return (to - from * coshd)/sinhd;
    }
    
}

vec4 transformPointToCurrentSpace(vec4 eucPoint) {
    if (curvature == 0.0) { //EUCLIDEAN
        return eucPoint;
    }
    
    vec3 P = eucPoint.xyz;
    float dist = sqrt(dot(P,P)) + 0.000001;
    vec4 v = vec4(P/dist, 0.0);

    if (curvature > 0.0) { //SPHERICAL
        return vec4(0.0,0.0,0.0,1.0) * cos(dist) + v * sin(dist);
    }
    if (curvature < 0.0) { //HYPERBOLIC
        return vec4(0.0,0.0,0.0,1.0) * cosh(dist) + v * sinh(dist);
    }
}
    

vec4 transformVectorToCurrentSpace(vec4 vector, vec4 point) {
	if (curvature == 0.0) return vector;
    float alpha = curvature;
	float x = point.x;
	float y = point.y;
	float z = point.z;
	float w = point.w;
    //vector * TransformMatrix(point)
	return vector * mat4(
		vec4(1.0 - (alpha * (x*x / (1.0 + w))),	-(alpha * (x*y / (1.0 + w))),		-(alpha * (x*z / (1.0 + w))),	-alpha * x),
		vec4(-(alpha * (y*x / (1.0 + w))),		1.0 - (alpha * (y*y / (1.0 + w))),	-(alpha * (y*z / (1.0 + w))),	-alpha * y),
		vec4(-(alpha * (z*x / (1.0 + w))),		-(alpha * (z*y / (1.0 + w))),		1.0 - (alpha * (z*z / (1.0 + w))),	-alpha * z),
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