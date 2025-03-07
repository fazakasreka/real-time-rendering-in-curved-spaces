#include <iostream>
#include "framework.h"

const int tessellationLevel = 20;

enum Direction
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	FORWARD,
	BACKWARD,
	NONE,
};

struct Camera { 
	vec4 eucPosition = vec4(0, 0.2, 2.0, 1.0f);
	vec4 velocity = vec4(0, 0, 0, 0);
	vec4 lookAt = vec4(0, 0, -1, 0);
	vec4 up =	 vec4(0, 1, 0, 0);

	float fov, asp, fp, bp;	

public:
	Camera() {
		asp = (float)windowWidth / windowHeight;
		fov = 90.0f * (float)M_PI / 180.0f;
		
		fp = 0.01f;

		if (curvature == SPH) 
			bp = 3.14f; 
		else 
			bp = 10.f;
	}

	void pan(float deltaX, float deltaY){ //x and y are in the range of -1 to 1
		vec3 lookAt3 = vec3(lookAt.x, lookAt.y, lookAt.z);
		vec3 up3 = vec3(up.x, up.y, up.z);
		vec3 right = cross(lookAt3, up3);

		lookAt3 = normalize(lookAt3 + deltaX * right + deltaY * up3);

		lookAt = vec4(lookAt3.x, lookAt3.y, lookAt3.z, 0);
	}



	void move(float dt, Direction move_direction){

			vec4 direction = vec4(0, 0, 0, 0);
			if (move_direction == LEFT){
				vec3 left = cross(vec3(up.x, up.y, up.z), vec3(lookAt.x, lookAt.y, lookAt.z));
				direction = vec4(left.x, left.y, left.z, 0);
			}  
			else if (move_direction == RIGHT){
				vec3 right = cross(vec3(lookAt.x, lookAt.y, lookAt.z), vec3(up.x, up.y, up.z));
				direction = vec4(right.x, right.y, right.z, 0);
			}
			else if (move_direction == UP)    direction = vec4(0, 1, 0, 0);
			else if (move_direction == DOWN)  direction = vec4(0, -1, 0, 0);
			else if (move_direction == FORWARD)  direction = lookAt;
			else if (move_direction == BACKWARD)  direction = -lookAt;

			eucPosition = eucPosition + direction * dt * 1.0f;

			if (curvature == SPH) { //we walked around int the spherical world
				vec4 sphPosition = transformPointToCurrentSpace(eucPosition);
				if(sphPosition.w < 0) {
					eucPosition = -eucPosition;
					up = -up;
				}
			} 
	}

	mat4 V() { // view matrix: translates the center to the origin
		if (curvature == EUC) {
			vec3 wVup = vec3(0, 1, 0);

			vec3 k_ = normalize(vec3(-lookAt.x, -lookAt.y, -lookAt.z));
			vec3 i_ = normalize(cross(wVup, k_));
			vec3 j_ = normalize(cross(k_, i_));

			return TranslateMatrix(eucPosition * oppositeVector()) * mat4(i_.x, j_.x, k_.x, 0,
				i_.y, j_.y, k_.y, 0,
				i_.z, j_.z, k_.z, 0,
				0, 0, 0, 1);
		
		}
		else {
			vec4 geomPosition = transformPointToCurrentSpace(eucPosition);

			vec4 lookAtTransformed = transformVectorToCurrentSpace(lookAt, geomPosition);
			vec4 wVup = transformVectorToCurrentSpace(up, geomPosition);


			float alpha = curvature;

			vec4 k_ = normalize(-lookAtTransformed);
			vec4 i_ = normalize(smartCross(geomPosition, wVup, k_)) * alpha;
			vec4 j_ = normalize(smartCross(geomPosition, k_, i_)) * alpha;

			return mat4(i_.x, j_.x, k_.x, alpha * geomPosition.x,
				i_.y, j_.y, k_.y, alpha * geomPosition.y,
				i_.z, j_.z, k_.z, alpha * geomPosition.z,
				alpha * i_.w, alpha * j_.w, alpha * k_.w, geomPosition.w);
		}

	}

	mat4 P() { // projection matrix: transforms the view frustum to the canonical view volume
		float A, B;

		if (curvature == EUC) {
			A = -(fp + bp) / (bp - fp);
			B = -2 *fp*bp / (bp - fp);
		}
		else {
			A = -smartSin(fp + bp) / smartSin(bp - fp);
			B = -2 * smartSin(fp)*smartSin(bp) / smartSin(bp - fp);
		}

		return mat4(1 / (tan(fov / 2)*asp), 0, 0, 0,
			0, 1 / tan(fov / 2), 0, 0,
			0, 0, A, -1,
			0, 0, B, 0);
		
	}

	
};

class GeomShader : public Shader {
public:
	GeomShader() {
		createShaderFromFiles("src/shaders/geom.vert", "src/shaders/geom.frag", "fragmentColor");
	}

	void Bind(RenderState state) {
		Use();      // make this program run
		
		setUniform(curvature, "curvature");

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

class Geometry {
protected:
	unsigned int vao, vbo;        // vertex array object
public:
	Geometry() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}
	virtual void Draw() = 0;
	~Geometry() {
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	void bindBuffer() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}
};

struct VertexData {
	vec4 position, normal;
	vec2 texcoord;
};

class ParamGeometry : public Geometry {
	unsigned int nVtxPerStrip, nStrips;
public:
	ParamGeometry() { nVtxPerStrip = nStrips = 0; }

	virtual void eval(Dnum2& U, Dnum2& V, Dnum2& X, Dnum2& Y, Dnum2& Z) = 0;

	VertexData GenVertexData(float u, float v) {
		VertexData vtxData;
		vtxData.texcoord = vec2(u, v);
		
		// Generate position and normal in Euclidean space
		Dnum2 X, Y, Z;
		Dnum2 U(u, vec2(1, 0)), V(v, vec2(0, 1));
		eval(U, V, X, Y, Z);
		
		vtxData.position = vec4(X.f, Y.f, Z.f, 1.0f);
		
		// Calculate normal using partial derivatives
		vec3 drdU(X.d.x, Y.d.x, Z.d.x);
		vec3 drdV(X.d.y, Y.d.y, Z.d.y);
		vec3 normal = normalize(cross(drdU, drdV));
		vtxData.normal = vec4(normal.x, normal.y, normal.z, 0.0f);
		
		return vtxData;
	}

	void create(int N = tessellationLevel, int M = tessellationLevel) {
		nVtxPerStrip = (M + 1) * 2;
		nStrips = N;
		std::vector<VertexData> vtxData;	// vertices on the CPU
		
		// Generate vertex data
		for (int i = 0; i < N; i++) {
			for (int j = 0; j <= M; j++) {
				vtxData.push_back(GenVertexData((float)j / M, (float)i / N));
				vtxData.push_back(GenVertexData((float)j / M, (float)(i + 1) / N));
			}
		}

		glBufferData(GL_ARRAY_BUFFER, nVtxPerStrip * nStrips * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);  // position
		glEnableVertexAttribArray(1);  // normal
		glEnableVertexAttribArray(2);  // texcoord
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texcoord));
	}

	void Draw() {
		glBindVertexArray(vao);
		for (unsigned int i = 0; i < nStrips; i++) {
			glDrawArrays(GL_TRIANGLE_STRIP, i * nVtxPerStrip, nVtxPerStrip);
		}
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
		if(curvature == SPH){
			Scale = ScaleMatrix(sph_scale);
		}else{
			Scale = ScaleMatrix(scale);
		}
		Rotate = RotationMatrix(rotationAngle, rotationAxis);
		Translate = TranslateMatrix(transformPointToCurrentSpace(translation));
	}

	void Draw(RenderState state) {
		if(curvature == SPH && !draw_in_spherical_space) {
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

	Camera camera;
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
			float height = y * 1.0f - 0.5f;  // Use consistent spacing
			plane_obj->translation = vec4(0.0f, height, 0.0f, 1.0f);
			plane_obj->scale = vec3(5.0f, 5.0f, 5.0f);

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
			vertical_plane_obj->scale = vec3(5.0f, 5.0f, 5.0f);
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
		light2.wLightPos = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		lights.push_back(light2);

		Light light3;
		light3.La = vec3(1.5f, 1.5f, 1.5f);
		light3.Le = vec3(3.0f, 3.0f, 3.0f);
		light3.wLightPos = vec4(0.0f, 0.0f, 2.0f, 1.0f);
		lights.push_back(light3);
	}

	void Render() {
		RenderState state;
		state.wEye = camera.eucPosition;
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
