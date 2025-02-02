#ifndef HYPERMATHS_H
#define HYPERMATHS_H

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>

#include "globalConstants.h"


inline float smartSin(float x) {
	if (curvature == HYP) return sinhf(x);
	return sinf(x);
}

inline float smartCos(float x) {
	if (curvature == HYP) return coshf(x);
	return cosf(x);
}

inline float smartArcCos(float x) {
	if (curvature == HYP) return acoshf(x);
	return acosf(x);

}

struct vec2 {
	float x, y;

	vec2(float x0 = 0, float y0 = 0) { x = x0; y = y0; }
	vec2 operator*(float a) const { return vec2(x * a, y * a); }
	vec2 operator/(float a) const { return vec2(x / a, y / a); }
	vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
	vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
	vec2 operator*(const vec2& v) const { return vec2(x * v.x, y * v.y); }
	vec2 operator-() const { return vec2(-x, -y); }
};

inline float dotEu(const vec2& v1, const vec2& v2) {
	return (v1.x * v2.x + v1.y * v2.y);
}

inline float length(const vec2& v) { return sqrtf(dotEu(v, v)); }

inline vec2 normalize(const vec2& v) { return v * (1 / length(v)); }

inline vec2 operator*(float a, const vec2& v) { return vec2(v.x * a, v.y * a); }

struct vec3 {
	float x, y, z;

	vec3(float x0 = 0, float y0 = 0, float z0 = 0) { x = x0; y = y0; z = z0; }
	vec3(vec2 v) { x = v.x; y = v.y; z = 0; }

	vec3 operator*(float a) const { return vec3(x * a, y * a, z * a); }
	vec3 operator/(float a) const { return vec3(x / a, y / a, z / a); }
	vec3 operator+(const vec3& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
	vec3 operator-(const vec3& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
	vec3 operator*(const vec3& v) const { return vec3(x * v.x, y * v.y, z * v.z); }
	vec3 operator-()  const { return vec3(-x, -y, -z); }

	float& operator[](int j) { return *(&x + j); }
	float operator[](int j) const { return *(&x + j); }
};

inline float dotEu(const vec3& v1, const vec3& v2) { return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z); }
inline float length(const vec3& v) { return sqrtf(dotEu(v, v)); }
inline vec3 normalize(const vec3& v) { return v * (1 / length(v)); }
inline vec3 cross(const vec3& v1, const vec3& v2) {
	return vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}
inline vec3 operator*(float a, const vec3& v) { return vec3(v.x * a, v.y * a, v.z * a); }


struct vec4 {
	float x, y, z, w;

	vec4(float x0 = 0, float y0 = 0, float z0 = 0, float w0 = 0) { x = x0; y = y0; z = z0; w = w0; }
	float& operator[](int j) { return *(&x + j); }
	float operator[](int j) const { return *(&x + j); }

	vec4 operator*(float a) const { return vec4(x * a, y * a, z * a, w * a); }
	vec4 operator/(float d) const { return vec4(x / d, y / d, z / d, w / d); }
	vec4 operator+(const vec4& v) const { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
	vec4 operator-(const vec4& v)  const { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
	vec4 operator*(const vec4& v) const { return vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
	void operator+=(const vec4 right) { x += right.x; y += right.y; z += right.z, w += right.z; }
	vec4 operator-()  const { return vec4(-x, -y, -z, -w); }
};

inline vec4 oppositeVector() {
	return vec4(-1, -1, -1, 1);
}

inline float dot(const vec4& v1, const vec4& v2) {
	if (curvature == HYP) return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z - v1.w * v2.w);
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w);
}

inline vec4 operator*(float a, const vec4& v) {
	return vec4(v.x * a, v.y * a, v.z * a, v.w * a);
}
inline float length(const vec4& v) { return sqrtf(dot(v, v)); }

inline vec4 normalize(const vec4& v) { return v * (1 / length(v)); }

inline float distance(vec4 p, vec4 q) {
	if (curvature == EUC) {
		vec4 direction = p - q;
			return length(direction);
	}
	else{
		return smartArcCos(-dot(q, p));
	}
}



struct mat3 { // row-major matrix 3x3
	vec3 rows[3];
public:
	mat3() {}
	mat3(float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22) {
		rows[0][0] = m00; rows[0][1] = m01; rows[0][2] = m02;
		rows[1][0] = m10; rows[1][1] = m11; rows[1][2] = m12;
		rows[2][0] = m20; rows[2][1] = m21; rows[2][2] = m22;
	}
	vec3& operator[](int i) { return rows[i]; }
	vec3 operator[](int i) const { return rows[i]; }
};

inline float det(const mat3& m) {
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

inline vec4 smartCross(const vec4& t, const vec4& a, const vec4& b) {
	float alph = curvature;
	mat3 mx(t.y, t.z, alph * t.w,
		a.y, a.z, alph * a.w,
		b.y, b.z, alph * b.w);
	mat3 my(t.x, t.z, alph * t.w,
		a.x, a.z, alph * a.w,
		b.x, b.z, alph * b.w);
	mat3 mz(t.x, t.y, alph * t.w,
		a.x, a.y, alph * a.w,
		b.x, b.y, alph * b.w);
	mat3 mw(t.x, t.y, t.z,
		a.x, a.y, a.z,
		b.x, b.y, b.z);
	return vec4(det(mx), -det(my), det(mz), -det(mw));
}



struct mat4 { // row-major matrix 4x4
	vec4 rows[4];
public:
	mat4() {}
	mat4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) {
		rows[0][0] = m00; rows[0][1] = m01; rows[0][2] = m02; rows[0][3] = m03;
		rows[1][0] = m10; rows[1][1] = m11; rows[1][2] = m12; rows[1][3] = m13;
		rows[2][0] = m20; rows[2][1] = m21; rows[2][2] = m22; rows[2][3] = m23;
		rows[3][0] = m30; rows[3][1] = m31; rows[3][2] = m32; rows[3][3] = m33;
	}
	mat4(vec4 it, vec4 jt, vec4 kt, vec4 ot) {
		rows[0] = it; rows[1] = jt; rows[2] = kt; rows[3] = ot;
	}

	vec4& operator[](int i) { return rows[i]; }
	vec4 operator[](int i) const { return rows[i]; }
	operator float*() const { return (float*)this; }
};

inline vec4 operator*(const vec4& v, const mat4& mat) {
	return v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2] + v[3] * mat[3];
}

inline mat4 operator*(const mat4& left, const mat4& right) {
	mat4 result;
	for (int i = 0; i < 4; i++) result.rows[i] = left.rows[i] * right;
	return result;
}

inline mat4 TranslateMatrix(vec4 position) {

	float alph = curvature;
	float x = position.x;
	float y = position.y;
	float z = position.z;
	float w = position.w;

	return mat4(
		vec4(1 - (alph * (x*x / (1 + w))),	-(alph * (x*y / (1 + w))),		-(alph * (x*z / (1 + w))),	-alph * x),
		vec4(-(alph * (y*x / (1 + w))),		1 - (alph * (y*y / (1 + w))),	-(alph * (y*z / (1 + w))),	-alph * y),
		vec4(-(alph * (z*x / (1 + w))),		-(alph * (z*y / (1 + w))),		1- (alph * (z*z / (1 + w))),	-alph * z),
		vec4(x,								y,								z,							w));
}

inline mat4 ScaleMatrix(vec3 s) {
	return mat4(vec4(s.x, 0,   0,   0),
			    vec4(0,   s.y, 0,   0),
				vec4(0,   0,   s.z, 0),
				vec4(0,   0,   0,   1));
}

inline mat4 RotationMatrix(float angle, vec3 w) {
	float c = cosf(angle), s = sinf(angle);
	w = normalize(w);
	return mat4(vec4(c * (1 - w.x*w.x) + w.x*w.x, w.x*w.y*(1 - c) + w.z*s, w.x*w.z*(1 - c) - w.y*s, 0),
			    vec4(w.x*w.y*(1 - c) - w.z*s, c * (1 - w.y*w.y) + w.y*w.y, w.y*w.z*(1 - c) + w.x*s, 0),
			    vec4(w.x*w.z*(1 - c) + w.y*s, w.y*w.z*(1 - c) - w.x*s, c * (1 - w.z*w.z) + w.z*w.z, 0),
			    vec4(0, 0, 0, 1));
}


inline vec4 transformVectorToCurrentSpace(float x, float y, float z, const vec4& point) {
	if (curvature == EUC) return vec4(x, y, z, 0.0f);
	return vec4(x, y , z, 0) * TranslateMatrix(point);
}
inline vec4 transformVectorToCurrentSpace(vec4& vector, vec4& point) {
	if (curvature == EUC) return vector;
	return vector * TranslateMatrix(point);
}

inline vec4 transformPointToCurrentSpace(float x, float y, float z) {

	if (curvature == EUC) return vec4(x, y, z, 1.0f);

	vec3 epoint(x, y, z);
	float dist = sqrtf(dotEu(epoint, epoint)) + 0.000001f;
	vec4 v(epoint.x / dist, epoint.y / dist, epoint.z / dist, 0);
	return vec4(0, 0, 0, 1) * smartCos(dist) + v * smartSin(dist);
}
inline vec4 transformPointToCurrentSpace(vec4& point) {

	if (curvature == EUC) return point;

	vec3 epoint(point.x, point.y, point.z);
	float dist = sqrtf(dotEu(epoint, epoint)) + 0.000001f;
	vec4 v(epoint.x / dist, epoint.y / dist, epoint.z / dist, 0);
	return vec4(0, 0, 0, 1) * smartCos(dist) + v * smartSin(dist);
}

template<class T> struct Dnum { // Dual numbers for automatic derivation
	float f; // function value
	T d;  // derivatives
	Dnum(float f0 = 0, T d0 = T(0)) { f = f0, d = d0; }
	Dnum operator+(Dnum r) { return Dnum(f + r.f, d + r.d); }
	Dnum operator-(Dnum r) { return Dnum(f - r.f, d - r.d); }
	Dnum operator*(Dnum r) {
		return Dnum(f * r.f, f * r.d + d * r.f);
	}
	Dnum operator/(Dnum r) {
		return Dnum(f / r.f, (r.f * d - r.d * f) / r.f / r.f);
	}
};

template<class T> Dnum<T> Exp(Dnum<T> g) { return Dnum<T>(expf(g.f), expf(g.f)*g.d); }
template<class T> Dnum<T> Sin(Dnum<T> g) { return  Dnum<T>(sinf(g.f), cosf(g.f)*g.d); }
template<class T> Dnum<T> Cos(Dnum<T>  g) { return  Dnum<T>(cosf(g.f), -sinf(g.f)*g.d); }
template<class T> Dnum<T> Tan(Dnum<T>  g) { return Sin(g) / Cos(g); }
template<class T> Dnum<T> Sinh(Dnum<T> g) { return  Dnum<T>(sinh(g.f), cosh(g.f)*g.d); }
template<class T> Dnum<T> Cosh(Dnum<T> g) { return  Dnum<T>(cosh(g.f), sinh(g.f)*g.d); }
template<class T> Dnum<T> Tanh(Dnum<T> g) { return Sinh(g) / Cosh(g); }
template<class T> Dnum<T> Log(Dnum<T> g) { return  Dnum<T>(logf(g.f), g.d / g.f); }
template<class T> Dnum<T> Pow(Dnum<T> g, float n) {
	return  Dnum<T>(powf(g.f, n), n * powf(g.f, n - 1) * g.d);
}

typedef Dnum<vec2> Dnum2;

#endif // HYPERMATHS_H
