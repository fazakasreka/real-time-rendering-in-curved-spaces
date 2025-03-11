#ifndef NON_EUCLIDEAN_MATHS_H
#define NON_EUCLIDEAN_MATHS_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include "framework.h"
#include "curvature.h"

inline float smartSin(float x) {
	if (Curvature::getCurvature() == HYP) return sinhf(x);
	return sinf(x);
}

inline float smartCos(float x) {
	if (Curvature::getCurvature() == HYP) return coshf(x);
	return cosf(x);
}

inline float smartArcCos(float x) {
	if (Curvature::getCurvature() == HYP) return acoshf(x);
	return acosf(x);

}

inline float smartDot(const vec4& v1, const vec4& v2) {
	if (Curvature::getCurvature() == HYP) return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z - v1.w * v2.w);
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w);
}

inline float smartLength(const vec4& v) { return sqrtf(smartDot(v, v)); }

inline vec4 smartNormalize(const vec4& v) { return v * (1 / smartLength(v)); }

inline float smartDistance(vec4 p, vec4 q) {
	if (Curvature::getCurvature() == EUC) {
		vec4 direction = p - q;
			return smartLength(direction);
	}
	else{
		return smartArcCos(-smartDot(q, p));
	}
}

inline vec4 smartCross(const vec4& t, const vec4& a, const vec4& b) {
	float alpha = Curvature::getCurvature();
	mat3 mx(t.y, t.z, alpha * t.w,
		a.y, a.z, alpha * a.w,
		b.y, b.z, alpha * b.w);
	mat3 my(t.x, t.z, alpha * t.w,
		a.x, a.z, alpha * a.w,
		b.x, b.z, alpha * b.w);
	mat3 mz(t.x, t.y, alpha * t.w,
		a.x, a.y, alpha * a.w,
		b.x, b.y, alpha * b.w);
	mat3 mw(t.x, t.y, t.z,
		a.x, a.y, a.z,
		b.x, b.y, b.z);
	return vec4(det(mx), -det(my), det(mz), -det(mw));
}

inline mat4 TranslateMatrix(vec4 position) {

	float alpha = Curvature::getCurvature();
	float x = position.x;
	float y = position.y;
	float z = position.z;
	float w = position.w;

	return mat4(
		vec4(1 - (alpha * (x*x / (1 + w))),	-(alpha * (x*y / (1 + w))),		-(alpha * (x*z / (1 + w))),	-alpha * x),
		vec4(-(alpha * (y*x / (1 + w))),		1 - (alpha * (y*y / (1 + w))),	-(alpha * (y*z / (1 + w))),	-alpha * y),
		vec4(-(alpha * (z*x / (1 + w))),		-(alpha * (z*y / (1 + w))),		1- (alpha * (z*z / (1 + w))),	-alpha * z),
		vec4(x,								y,								z,							w));
}

inline vec4 transformVectorToCurrentSpace(float x, float y, float z, const vec4& point) {
	if (Curvature::getCurvature() == EUC) return vec4(x, y, z, 0.0f);
	return vec4(x, y , z, 0) * TranslateMatrix(point);
}
inline vec4 transformVectorToCurrentSpace(vec4& vector, vec4& point) {
	if (Curvature::getCurvature() == EUC) return vector;
	return vector * TranslateMatrix(point);
}

inline vec4 transformPointToCurrentSpace(float x, float y, float z) {

	if (Curvature::getCurvature() == EUC) return vec4(x, y, z, 1.0f);

	vec3 eucPoint(x, y, z);
	float dist = sqrtf(euclideanDot(eucPoint, eucPoint)) + 0.000001f;
	vec4 v(eucPoint.x / dist, eucPoint.y / dist, eucPoint.z / dist, 0);
	return vec4(0, 0, 0, 1) * smartCos(dist) + v * smartSin(dist);
}
inline vec4 transformPointToCurrentSpace(vec4& point) {

	if (Curvature::getCurvature() == EUC) return point;

	vec3 eucPoint(point.x, point.y, point.z);
	float dist = sqrtf(euclideanDot(eucPoint, eucPoint)) + 0.000001f;
	vec4 v(eucPoint.x / dist, eucPoint.y / dist, eucPoint.z / dist, 0);
	return vec4(0, 0, 0, 1) * smartCos(dist) + v * smartSin(dist);
}

#endif // NON_EUCLIDEAN_MATHS_H