#ifndef SHADER_H
#define SHADER_H
#include "gpuProgram.h"
#include "texture.h"

struct Material {
	vec3 kd, ks, ka;
	float shininess, emission;

	Material() { ka = vec3(1, 1, 1), kd = ks = vec3(0, 0, 0); shininess = 1; emission = 0; }
};

struct Light {
	vec3 La, Le;
	vec4 wLightPos; // homogeneous coordinates, can be at ideal point
};

struct RenderState {
	mat4	           MVP, Scale, Rotate, Translate, Minv, V, P;
	Material *         material;
	std::vector<Light> lights;
	Texture *          texture;
	vec4	           wEye;
};

class Shader : public GPUProgram {
public:
	virtual void Bind(RenderState state) = 0;

    void setUniformMaterial(const Material &material, const std::string &name);
    void setUniformLight(const Light &light, const std::string &name);
    void setUniformMaterial(const Material *material, const std::string &name);
    void createShaderFromFiles(const char* vertPath, const char* fragPath, const char* outputName);
};

#endif // SHADER_H