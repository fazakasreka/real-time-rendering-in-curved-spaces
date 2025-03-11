#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include "frameworkMath.h"

const int tessellationLevel = 20;

struct VertexData {
	vec4 position, normal;
	vec2 texcoord;
};

class Geometry {
protected:
	unsigned int vao, vbo;        // vertex array object
public:
	Geometry();
	virtual ~Geometry();
	virtual void Draw() = 0;
	void bindBuffer();
};

class ParamGeometry : public Geometry {
protected:
	unsigned int nVtxPerStrip, nStrips;
public:
	ParamGeometry();
	virtual void eval(Dnum2& U, Dnum2& V, Dnum2& X, Dnum2& Y, Dnum2& Z) = 0;
	VertexData GenVertexData(float u, float v);
	void create(int N = tessellationLevel, 
				int M = tessellationLevel);
	void Draw() override;
};

#endif // GEOMETRY_H