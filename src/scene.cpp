#include <iostream>
#include "framework.h"
#include "nonEuclidean.h"

class GeomShader : public Shader {
public:
	GeomShader() {
		createShaderFromFiles("src/shaders/geom.vert", "src/shaders/geom.frag", "fragmentColor");
	}

	void Bind(RenderState state) {
		Use();      // make this program run
		
		setUniform(Curvature::getCurvature(), "curvature");

		setUniform(state.Scale, "ScaleMatrix");
		setUniform(state.Rotate, "RotateMatrix");
		setUniform(state.Translate, "TranslateMatrix");
		setUniform(state.VP, "VPMatrix");

		setUniform(state.wEye, "wEye");

		setUniform(*state.texture, std::string("diffuseTexture"));
		setUniformMaterial(state.material, "material");

		setUniform((int)state.lights.size(), "nLights");
		for (unsigned int i = 0; i < state.lights.size(); i++) {
			setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
		}
	}
};

class CheckerBoardTexture : public Texture {
public:
	CheckerBoardTexture(const int width, const int height) : Texture() {
		std::vector<vec4> image(width * height);
		const vec4 yellow(1, 1, 0, 1), blue(0, 0, 1, 1);
		for (int x = 0; x < width; x++) for (int y = 0; y < height; y++) {
			image[y * width + x] = (x & 1) ^ (y & 1) ? yellow : blue;
		}
		create(width, height, image, GL_NEAREST);
	}
};

class Sphere : public ParamGeometry {
public:
	Sphere() { create(); }
	void eval(Dnum2& U, Dnum2& V, Dnum2& X, Dnum2& Y, Dnum2& Z) {
		U = U * 2.0f * (float)M_PI, V = V * (float)M_PI;
		X = Cos(U) * Sin(V); Y = Sin(U) * Sin(V); Z = Cos(V);
	}
};

class Plane : public ParamGeometry {
private:
	float height;
	float width;
	float depth;


public:
	Plane() { 
		create(); 
	}

	void eval(Dnum2& U, Dnum2& V, Dnum2& X, Dnum2& Y, Dnum2& Z) override {
		U = U - 0.5f;
		V = V - 0.5f;
		
		X = U;
		Y = Dnum2(0.0f);  // Constant height
		Z = V;
	}
};


struct Object {
	Shader *   shader;
	Material * material;
	Texture *  texture;
	Geometry * geometry;

	vec4 translation = vec4(0, 0, 0, 0);
	vec3 rotationAxis = vec3(0, 0, 1);
	vec3 scale = vec3(1, 1, 1);
	vec3 sph_scale = vec3(1, 1, 1);
	float rotationAngle = 0;

	bool draw_in_spherical_space = true;

public:
	Object(
		Shader * _shader, 
		Material * _material,
		Texture * _texture, 
		Geometry * _geometry
	) :
		translation(vec4(0, 0, 0, 1.0)), 
		rotationAxis(0, 0, 1), 
		rotationAngle(0), 
		scale(1, 1, 1) {

		shader = _shader;
		texture = _texture;
		material = _material;
		geometry = _geometry;
	}

	virtual void SetModelingTransform(mat4& Scale, mat4& Rotate, mat4& Translate) {
		if(Curvature::isSpherical()){
			Scale = ScaleMatrix(sph_scale);
		}else{
			Scale = ScaleMatrix(scale);
		}
		Rotate = RotationMatrix(rotationAngle, rotationAxis);
		Translate = TranslateMatrix(transformPointToCurrentSpace(translation));
	}

	void Draw(RenderState state) {
		if(Curvature::isSpherical() && !draw_in_spherical_space) {
			return;
		}
		mat4 Scale, Rotate, Translate;
		SetModelingTransform(Scale, Rotate, Translate);
		state.Scale = Scale;	
		state.Rotate = Rotate;
		state.Translate = Translate;
		state.VP = state.V * state.P;
		state.material = material;
		state.texture = texture;
		shader->Bind(state);
		geometry->Draw();
	}

	virtual void Animate(float tstart, float tend) { }
};

class Scene {
	std::vector<Object *> objects;
	std::vector<Light> lights;
public:

	GeomCamera camera;
	void Build() {
		// Shaders
		Shader * geomShader = new GeomShader();

		// Material
		Material * material = new Material;
		material->kd = vec3(0.5f, 0.1f, 0.1f);
		material->ks = vec3(0.5, 0.1,  0.1);
		material->ka = vec3(0.5f, 0.1f, 0.1f);
		material->shininess = 100;


		// Textures
		Texture * texture4x4 = new CheckerBoardTexture(4, 4);
		Texture * texture40x40 = new CheckerBoardTexture(40, 40);

		// Geometries
		Sphere * sphere_geom = new Sphere();
		Plane * plane_geom = new Plane();
		
		// Planes grid
		for (int y = -3; y <= 3; y++) {
			
			// horizontal plane
			Object * plane_obj = new Object(
				geomShader, 
				material, 
				texture40x40, 
				plane_geom
			);
			float height = y;  // Use consistent spacing
			plane_obj->translation = vec4(0.0f, height, 0.0f, 1.0f);
			plane_obj->scale = vec3(6.0f, 6.0f, 6.0f);

			plane_obj->draw_in_spherical_space = y == 0;
			plane_obj->sph_scale = vec3(3.14f, 3.14f, 3.14f);

			objects.push_back(plane_obj);

			//vertical plane
			Object * vertical_plane_obj = new Object(
				geomShader, 
				material, 
				texture40x40,
				plane_geom
			);
			vertical_plane_obj->rotationAxis = vec3(0, 0, 1);
			vertical_plane_obj->rotationAngle = M_PI / 2.0f;
			vertical_plane_obj->translation = vec4(height, 0.0f, 0.0f, 1.0f);
			vertical_plane_obj->scale = vec3(6.0f, 6.0f, 6.0f);
			vertical_plane_obj->draw_in_spherical_space = false;
			objects.push_back(vertical_plane_obj);
		
		}
		
		// Grid of spheres
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				Object * sphere_obj = new Object(geomShader, material, texture4x4, sphere_geom);
				sphere_obj->translation = vec4(i * 1.57f, 0.0f, j * 1.57f, 1.0f);
				sphere_obj->scale = vec3(0.3f, 0.3f, 0.3f);
				sphere_obj->sph_scale = vec3(0.3f, 0.3f, 0.3f);
				objects.push_back(sphere_obj);
			}
		}

		//Lights
		Light light;
		light.La = vec3(1.5f, 1.5f, 1.5f);
		light.Le = vec3(3.0f, 3.0f, 3.0f);
		light.wLightPos = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		lights.push_back(light);

		Light light2;
		light2.La = vec3(1.5f, 1.5f, 1.5f);
		light2.Le = vec3(3.0f, 3.0f, 3.0f);
		light2.wLightPos = vec4(0.0f, 3.0f, 0.0f, 1.0f);
		lights.push_back(light2);

		Light light3;
		light3.La = vec3(1.5f, 1.5f, 1.5f);
		light3.Le = vec3(3.0f, 3.0f, 3.0f);
		light3.wLightPos = vec4(0.0f, 0.0f, 2.0f, 1.0f);
		lights.push_back(light3);
	}

	void Render() {
		RenderState state;
		state.wEye = camera.getPosition();
		state.V = camera.V();
		state.P = camera.P();
		state.lights = lights;
		for (auto * obj : objects) {
			if (dynamic_cast<GeomShader*>(obj->shader)) {
				obj->Draw(state);
			}
		}
	}

	void Animate(float tstart, float tend) {
		for (Object * obj : objects) {
			obj->Animate(tstart, tend);
		}

	}
};
