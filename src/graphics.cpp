#include <iostream>
#include "framework.h"

const int tessellationLevel = 20;
float wMirror = 1;

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
	vec4 position = vec4(0, 0, 0.0, 1.0f);
	vec4 velocity = vec4(0, 0, 0, 0);
	vec4 lookAt = vec4(0, 0, -1, 0);
	vec4 up =	 vec4(0, 1, 0, 0);

	float fov, asp, fp, bp;	

public:
	Camera() {
		asp = (float)windowWidth / windowHeight;
		fov = 90.0f * (float)M_PI / 180.0f;
		fp = 0.1f;  // Near plane
		bp = (curvature == SPH) ? 3.14f : 30.0f;  // Far plane different for spherical
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
			
			if (curvature == EUC) {
				position = position + direction * dt *1.0f;
				return;
			}

			direction = transformVectorToCurrentSpace(direction, position);

			if (!(direction.x == 0 && direction.y == 0 && direction.z == 0 && direction.w == 0)) {
				position = position * smartCos(dt / 1.0f) + direction * smartSin(dt / 1.0f);
			}

			if (curvature == SPH && position.w < 0.0f) { //we walked around int the spherical world
				position = -position;
				up = -up;
			} 
	}

	mat4 V() { // view matrix: translates the center to the origin
		if (curvature == EUC) {
			vec3 wVup = vec3(0, 1, 0);

			vec3 k_ = normalize(vec3(-lookAt.x, -lookAt.y, -lookAt.z));
			vec3 i_ = normalize(cross(wVup, k_));
			vec3 j_ = normalize(cross(k_, i_));

			return TranslateMatrix(position * oppositeVector()) * mat4(i_.x, j_.x, k_.x, 0,
				i_.y, j_.y, k_.y, 0,
				i_.z, j_.z, k_.z, 0,
				0, 0, 0, 1);
		
		}
		else {
			vec4 lookAtTransformed = transformVectorToCurrentSpace(lookAt, position);
			vec4 wVup = transformVectorToCurrentSpace(up, position);


			float alpha = curvature;

			vec4 k_ = normalize(-lookAtTransformed);
			vec4 i_ = normalize(smartCross(position, wVup, k_)) * alpha;
			vec4 j_ = normalize(smartCross(position, k_, i_)) * alpha;

			return mat4(i_.x, j_.x, k_.x, alpha * position.x,
				i_.y, j_.y, k_.y, alpha * position.y,
				i_.z, j_.z, k_.z, alpha * position.z,
				alpha * i_.w, alpha * j_.w, alpha * k_.w, position.w);
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
		int sign = (curvature == HYP) ? -1 : 1;
		setUniform(sign, "LorentzSign");
		setUniform(wMirror, "wMirror");

		mat4 RotateMatrix = state.M;
		RotateMatrix[3][0] = RotateMatrix[3][1] = RotateMatrix[3][2] = 0;
		setUniform(RotateMatrix, "RotateMatrix");

		setUniform(vec4(state.M[3][0], state.M[3][1], state.M[3][2], 1), "eucTranslate");

		setUniform(state.wEye, "wEye");
		setUniform(vec4(state.V[0][0], state.V[1][0], state.V[2][0], 0), "ic");
		setUniform(vec4(state.V[0][1], state.V[1][1], state.V[2][1], 0), "jc");
		setUniform(vec4(state.V[0][2], state.V[1][2], state.V[2][2], 0), "kc");

		setUniform(state.P[0][0], "sFovX");
		setUniform(state.P[1][1], "sFovY");
		float alpha = state.P[2][2], beta = state.P[3][2];
		setUniform(beta / (alpha - 1), "fp");
		setUniform(beta / (alpha + 1), "bp");

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
		
		// Generate vertex data in parameter space
		for (int i = 0; i < N; i++) {
			for (int j = 0; j <= M; j++) {
				vtxData.push_back(GenVertexData((float)j / M, (float)i / N));
				vtxData.push_back(GenVertexData((float)j / M, (float)(i + 1) / N));
			}
		}

		// Scale the geometry (now in Euclidean space)
		for (auto& vtx : vtxData) {
			vtx.position = vtx.position * ScaleMatrix(vec3(0.1, 0.1, 0.1));
		}

		// Set up OpenGL buffers
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
	Plane(float _height = 0.0f, float _width = 2.0f * (float)M_PI, float _depth = 2.0f * (float)M_PI) { 
		height = _height;
		width = _width;
		depth = _depth;
		create(); 
	}

	void eval(Dnum2& U, Dnum2& V, Dnum2& X, Dnum2& Y, Dnum2& Z) override {
		U = U * width - width / 2.0f;
		V = V * depth - depth / 2.0f;
		
		X = U;
		Y = Dnum2(height);  // Constant height
		Z = V;
	}
};


struct Object {
	Shader *   shader;
	Material * material;
	Texture *  texture;
	Geometry * geometry;

	vec4 translation;
	vec3 rotationAxis;
	float rotationAngle;

public:
	Object(Shader * _shader, Material * _material, Texture * _texture, Geometry * _geometry) :
		translation(vec4(0, 0, 0, 1.0)), rotationAxis(0, 0, 1), rotationAngle(0) {

		shader = _shader;
		texture = _texture;
		material = _material;
		geometry = _geometry;
	}

	virtual void SetModelingTransform(mat4& M, mat4& Minv) {
		M = RotationMatrix(rotationAngle, rotationAxis) * TranslateMatrix(translation);
		if (true) {
			Minv = TranslateMatrix(translation * oppositeVector()) * RotationMatrix(-rotationAngle, rotationAxis);
		} else {
			Minv = TranslateMatrix(-translation) * RotationMatrix(-rotationAngle, rotationAxis);
		}
	}

	void Draw(RenderState state) {
		mat4 M, Minv;
		SetModelingTransform(M, Minv);
		state.M = M;
		state.Minv = Minv;
		state.MVP = state.M * state.V * state.P;
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

		// Materials
		Material * material0 = new Material;
		material0->kd = vec3(0.5f, 0.1f, 0.1f);
		material0->ks = vec3(0.5, 0.1,  0.1);
		material0->ka = vec3(0.5f, 0.1f, 0.1f);
		material0->shininess = 100;

		Material * mater1 = new Material;
		mater1->kd = vec3(0.1f, 0.3f, 0.3f);
		mater1->ks = vec3(0.1f, 0.3f, 0.3f);
		mater1->ka = vec3(0.1f, 0.3f, 0.3f);
		mater1->shininess = 30;

		Material * mater2 = new Material;
		mater2->kd = vec3(0.3f, 0.1f, 0.3f);
		mater2->ks = vec3(0.3f, 0.1f, 0.3f);
		mater2->ka = vec3(0.3f, 0.1f, 0.3f);
		mater2->shininess = 30;

		Material * mater3 = new Material;
		mater3->kd = vec3(0.3f, 0.3f, 0.5f);
		mater3->ks = vec3(0.3f, 0.3f, 0.5f);
		mater3->ka = vec3(0.3f, 0.3f, 0.5f);
		mater3->shininess = 30;

		std::vector<Material*> materials;
		materials.push_back(mater1);
		materials.push_back(mater2);
		materials.push_back(mater3);

		// Scene parameters
		float gridSize = 1.57f;  // π/2 for consistent spacing
		int N = 1;              // grid size (half-width)
		int Ny = 3;            // number of vertical levels (half-height)

		// Textures
		Texture * texture4x8 = new CheckerBoardTexture(4, 8);
		Texture * texturePlane = new CheckerBoardTexture(40, 40);

		// Geometries
		Sphere * sphere_geom = new Sphere();
		Plane * plane_geom = new Plane(
			0.0f, 
			SPH ? 2.0f * (float)M_PI : 30.0f, 
			SPH ? 2.0f * (float)M_PI : 30.0f);
		
		// Create horizontal planes at different heights
		for (int y = -Ny; y <= Ny; y++) {
			int mat = y + Ny < materials.size() ? y + Ny : 0;
			
			// Create horizontal plane
			Object * plane_obj = new Object(geomShader, materials[mat], texturePlane, plane_geom);
			float height = y * 1.0f;  // Use consistent spacing
			plane_obj->translation = vec4(0.0f, height, 0.0f, 1.0f);
			objects.push_back(plane_obj);

			// Create vertical plane (except for spherical geometry)
			if (curvature != SPH) {
				Object * planeV = new Object(geomShader, materials[mat], texturePlane, plane_geom);
				planeV->rotationAxis = vec3(0, 0, 1);
				planeV->rotationAngle = M_PI / 2.0f;  // 90 degrees
				planeV->translation = vec4(height, 0.0f, 0.0f, 1.0f);
				objects.push_back(planeV);
			}
		}
		
		// Create grid of spheres
		for (int i = -N; i <= N; i++) {
			for (int j = -N; j <= N; j++) {
				Object * sphere_obj = new Object(geomShader, material0, texture4x8, sphere_geom);
				sphere_obj->translation = vec4(i * gridSize, 0.0f, j * gridSize, 1.0f);
				objects.push_back(sphere_obj);
			}
		}

		// Create lights in Euclidean coordinates
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
		state.wEye = camera.position;
		state.V = camera.V();
		state.P = camera.P();
		state.lights = lights;
		for (auto * obj : objects) {
			if (dynamic_cast<GeomShader*>(obj->shader)) {
				wMirror = 1;
				obj->Draw(state);
				if (curvature == SPH) {
					wMirror = -1;
					obj->Draw(state);
				}
			}
		}
	}

	void Animate(float tstart, float tend) {
		for (Object * obj : objects) {
			obj->Animate(tstart, tend);
		}

	}
};
