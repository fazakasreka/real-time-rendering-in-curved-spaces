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

		fp = 0.1f; 

		if (curvature == SPH) 
			bp = 3.14f; 
		else 
			bp = 30.f;
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
	//---------------------------
	const char * vertexSource = R"(
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
	)";

	const char * fragmentSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		struct Material {
			vec3 kd, ks, ka;
			float shininess, emission;
		};

		uniform Material material;
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform sampler2D diffuseTexture;
		uniform float LorentzSign;
		in  vec4 wNormal;       // interpolated world sp normal
		in  vec4 wView;         // interpolated world sp view
		in  vec4 wLight[8];     // interpolated world sp illum dir
		in  vec2 texcoord;
        out vec4 fragmentColor; // output goes to frame buffer

		float dotGeom(vec4 u, vec4 v) {
			return u.x * v.x + u.y * v.y + u.z * v.z + LorentzSign * u.w * v.w;
		}

		void main() {
			vec4 N = normalize(wNormal);
			vec4 V = normalize(wView); 
			vec3 texColor = texture(diffuseTexture, texcoord).rgb;
			vec3 ka = material.ka * texColor;
			vec3 kd = material.kd * texColor;

			vec3 radiance = texColor * material.emission;
			for(int i = 0; i < nLights; i++) {
				vec4 L = normalize(wLight[i]);
				vec4 H = normalize(L + V);
				float cost = max(dotGeom(N, L), 0), cosd = max(dotGeom(N, H), 0);
				// kd and ka are modulated by the texture
				radiance += ka * lights[i].La + (kd * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}
			fragmentColor = vec4(radiance, 1);
		}
	)";
public:
	GeomShader() {
		create(vertexSource, fragmentSource, "fragmentColor");
	}

	void Bind(RenderState state) {
		Use(); 		// make this program run
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

class HyperbolicShader : public Shader {
	//---------------------------
	const char * vertexSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		uniform mat4  MVP, M, Minv; // MVP, Model, Model-inverse
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform vec4  wEye;         // pos of eye
		uniform float kappa;
		uniform float wMirror;
		layout(location = 0) in vec4  vtxPos;            // pos in modeling space
		layout(location = 1) in vec4  vtxNorm;      	 // normal in modeling space
		layout(location = 2) in vec2  vtxUV;
		out vec4 wNormal;		    // normal in world space
		out vec4 wView;             // view in world space
		out vec4 wLight[8];		    // light dir in world space
		out vec2 texcoord;

		float dotLorentz(vec4 u, vec4 v) {
			float d = u.x * v.x + u.y * v.y + u.z * v.z;
			return (kappa < 0) ? d - u.w * v.w : d + u.w * v.w;
		}

		vec4 direction(vec4 to, vec4 from) {
			if (kappa > 0) {
				float cosd = dot(from, to);
				float sind = sqrt(1 - cosd * cosd);
				return (to - from * cosd)/sind;
			}
			if (kappa < 0) {
				float coshd = -dotLorentz(from, to);
				float sinhd = sqrt(coshd * coshd - 1);
				return (to - from * coshd)/sinhd;
			}
			return normalize(to - from);
		}
		
		void main() {
			gl_Position = wMirror * vtxPos * MVP; // to NDC
			vec4 wPos = vtxPos * M;

			for(int i = 0; i < nLights; i++) wLight[i] = direction(lights[i].wLightPos, wPos);

		    wView  = direction(wEye, wPos);
			if (kappa == 0) {
				wNormal = Minv * vtxNorm;
				wNormal.w = 0;
			} else {
				wNormal = vtxNorm * M;
			}

		    texcoord = vtxUV;
		}
	)";

	const char * fragmentSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};

		struct Material {
			vec3 kd, ks, ka;
			float shininess, emission;
		};

		uniform Material material;
		uniform Light[8] lights;    // light sources 
		uniform int   nLights;
		uniform sampler2D diffuseTexture;
		in  vec4 wNormal;       // interpolated world sp normal
		in  vec4 wView;         // interpolated world sp view
		in  vec4 wLight[8];     // interpolated world sp illum dir
		in  vec2 texcoord;
 		uniform float kappa;
        out vec4 fragmentColor; // output goes to frame buffer

		float dotGeom(vec4 u, vec4 v) {
			float d = u.x * v.x + u.y * v.y + u.z * v.z;
			return (kappa < 0) ? d - u.w * v.w : d + u.w * v.w;
		}

		void main() {
			vec4 N = normalize(wNormal);
			vec4 V = normalize(wView); 
			vec3 texColor = texture(diffuseTexture, texcoord).rgb;
			if(texColor.x == 1 && texColor.y == 0) texColor = vec3(1,1,1);
			vec3 ka = material.ka * texColor;
			vec3 kd = material.kd * texColor;

			vec3 radiance = texColor * material.emission;
			for(int i = 0; i < nLights; i++) {
				vec4 L = normalize(wLight[i]);
				vec4 H = normalize(L + V);
				float cost = max(dotGeom(N, L), 0), cosd = max(dotGeom(N, H), 0);
				// kd and ka are modulated by the texture
				radiance += ka * lights[i].La + (kd * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}
			fragmentColor = vec4(radiance, 1);
		}
	)";
public:
	HyperbolicShader() {
		create(vertexSource, fragmentSource, "fragmentColor");
	}

	void Bind(RenderState state) {
		Use(); 		// make this program run
		setUniform(curvature, "kappa");
		setUniform(wMirror, "wMirror");
		setUniform(state.MVP, "MVP");
		setUniform(state.M, "M");
		setUniform(state.Minv, "Minv");

		vec4 wEyeRender = state.wEye;
		setUniform(wEyeRender, "wEye");

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
		Dnum2 X, Y, Z;
		Dnum2 U(u, vec2(1, 0)), V(v, vec2(0, 1));
		eval(U, V, X, Y, Z);
		vtxData.position = vec4(X.f, Y.f, Z.f, 1.0f);
		vec4 drdU(X.d.x, Y.d.x, Z.d.x, 0.0f), drdV(X.d.y, Y.d.y, Z.d.y, 0.0f);
		vtxData.normal = smartCross(vec4(0.0f, 0.0f, 0.0f, 1.0f),drdU, drdV);
		return vtxData;
	}

	void create(int N = tessellationLevel, int M = tessellationLevel) {
		nVtxPerStrip = (M + 1) * 2;
		nStrips = N;
		std::vector<VertexData> vtxData;	// vertices on the CPU
		for (int i = 0; i < N; i++) {
			for (int j = 0; j <= M; j++) {
				vtxData.push_back(GenVertexData((float)j / M, (float)i / N));
				vtxData.push_back(GenVertexData((float)j / M, (float)(i + 1) / N));
			}
		}
		for (int i = 0; i < vtxData.size(); i++) {
			vec4 scaledPos = vtxData[i].position * ScaleMatrix(vec3(0.1, 0.1, 0.1));
			vtxData[i].position = transformPointToCurrentSpace(scaledPos);
			vtxData[i].normal = transformVectorToCurrentSpace(vtxData[i].normal, vtxData[i].position);
		}
		glBufferData(GL_ARRAY_BUFFER, nVtxPerStrip * nStrips * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);  // attribute array 0 = POSITION
		glEnableVertexAttribArray(1);  // attribute array 1 = NORMAL
		glEnableVertexAttribArray(2);  // attribute array 2 = TEXCOORD0
		// attribute array, components/attribute, component type, normalize?, stride, offset
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texcoord));
	}

	void Draw() {
		glBindVertexArray(vao);
		for (unsigned int i = 0; i < nStrips; i++) glDrawArrays(GL_TRIANGLE_STRIP, i *  nVtxPerStrip, nVtxPerStrip);
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

class Plane : public Geometry {
public:

	float a = 3.14f;
	float b = 3.14f;
	float height = 0.0f;

	std::vector<VertexData> vtxData;

	void create() {
		glBufferData(GL_ARRAY_BUFFER, vtxData.size() * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);  // attribute array 0 = POSITION
		glEnableVertexAttribArray(1);  // attribute array 1 = NORMAL
		glEnableVertexAttribArray(2);  // attribute array 2 = TEXCOORD0
		// attribute array, components/attribute, component type, normalize?, stride, offset
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texcoord));
	}

	Plane(float y_= 0.0f, float a_ = 3.14f, float b_= 3.14f) {
		height = y_;
		a = a_;
		b = b_;

		int N = 50;
		int M = 50;

		if (curvature == SPH) {
			N = 10;
			M = 10;
		}
		for (int i = -N/2; i <= N/2; i++) {
			for (int j = -M/2; j <= M/2; j++) {
				vec4 x = vec4(b/N*i ,	  height,	a/M*j, 1.0f);	vec4 y = vec4(b/N*i,	  height, a/M*(j+1), 1.0f);

				vec4 z = vec4(b/N*(i+1), height,	a/M *j, 1.0f);		vec4 w = vec4(b/N*(i+1),	height, a/M*(j+1), 1.0f);

				VertexData Xvtx;
				Xvtx.position = x;
				Xvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Xvtx.texcoord = vec2(x.x/ b, x.y / a);

				VertexData Yvtx;
				Yvtx.position = y;
				Yvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Yvtx.texcoord = vec2(y.x / b, y.y / a);

				VertexData Zvtx;
				Zvtx.position = z;
				Zvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Zvtx.texcoord = vec2(z.x / b, z.y / a);

				VertexData Wvtx;
				Wvtx.position = w;
				Wvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Wvtx.texcoord = vec2(w.x / b, w.y / a);

				vtxData.push_back(Xvtx);
				vtxData.push_back(Yvtx);
				vtxData.push_back(Zvtx);

				if (curvature == SPH) {
					vtxData.push_back(Zvtx);
					vtxData.push_back(Yvtx);
					vtxData.push_back(Wvtx);
				}
			}
		}

		if (curvature != EUC) toBent();

		create();
	}

	void toBent() {
		for (int i = 0; i < vtxData.size(); i++) {
			vtxData[i].position = transformPointToCurrentSpace(vtxData[i].position);
			vtxData[i].normal = transformVectorToCurrentSpace(vtxData[i].normal, vtxData[i].position);
		}
	}

	void Draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vtxData.size());
	}

};

class VerticalPlane : public Geometry {
public:

	float a = 3.14f;
	float b = 3.14f;
	float width = 0.0f;

	std::vector<VertexData> vtxData;

	void create() {
		glBufferData(GL_ARRAY_BUFFER, vtxData.size() * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);  // attribute array 0 = POSITION
		glEnableVertexAttribArray(1);  // attribute array 1 = NORMAL
		glEnableVertexAttribArray(2);  // attribute array 2 = TEXCOORD0
		// attribute array, components/attribute, component type, normalize?, stride, offset
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texcoord));
	}

	VerticalPlane(float y_ = 0.0f, float a_ = 3.14f, float b_ = 3.14f) {
		width = y_;
		a = a_;
		b = b_;

		int N = 50;
		int M = 50;

		if (curvature == SPH) {
			N = 10;
			M = 10;
		}
		for (int i = -N / 2; i <= N / 2; i++) {
			for (int j = -M / 2; j <= M / 2; j++) {
				vec4 x = vec4( width, b / N * i, a / M * j, 1.0f);	vec4 y = vec4( width, b / N * i, a / M * (j + 1), 1.0f);

				vec4 z = vec4(width, b / N * (i + 1), a / M * j, 1.0f);		vec4 w = vec4(width, b / N * (i + 1),  a / M * (j + 1), 1.0f);

				VertexData Xvtx;
				Xvtx.position = x;
				Xvtx.normal = vec4(1.0f, 0.0f, 0.0f, 0.0f);
				Xvtx.texcoord = vec2(x.x / b, x.y / a);

				VertexData Yvtx;
				Yvtx.position = y;
				Yvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Yvtx.texcoord = vec2(y.x / b, y.y / a);

				VertexData Zvtx;
				Zvtx.position = z;
				Zvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Zvtx.texcoord = vec2(z.x / b, z.y / a);

				VertexData Wvtx;
				Wvtx.position = w;
				Wvtx.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Wvtx.texcoord = vec2(w.x / b, w.y / a);

				vtxData.push_back(Xvtx);
				vtxData.push_back(Yvtx);
				vtxData.push_back(Zvtx);

				if (curvature == SPH) {
					vtxData.push_back(Zvtx);
					vtxData.push_back(Xvtx);
					vtxData.push_back(Wvtx);
				}
			}
		}

		if (curvature != EUC) toBent();

		create();
	}

	void toBent() {
		for (int i = 0; i < vtxData.size(); i++) {
			vtxData[i].position = transformPointToCurrentSpace(vtxData[i].position);
			vtxData[i].normal = transformVectorToCurrentSpace(vtxData[i].normal, vtxData[i].position);
		}
	}

	void Draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vtxData.size());
	}

};

class Line: public Geometry {
	float POINT_DISTANCE = 0.01f;
public:
	std::vector<vec4> vtx;
	int numberOfVerticies;

	Line(vec4 start, vec4 end) {

		if (curvature == EUC) {
			vec4 vector = end - start;
			float len = length(vector);
			vec4 dir = normalize(vector);

			int numberOfPoints = len / POINT_DISTANCE;
			vtx.push_back(start);

			for (float t = 0; t < len; t+=POINT_DISTANCE) {
				vtx.push_back(start + t *dir);
			}
			vtx.push_back(end);
		
		}
		else {
			float d = distance(start, end);
			
			for (float t = 0; t < d; t += POINT_DISTANCE) {
				vec4 point = start * (smartSin((1-t)*d) / smartSin(d) )  +   end * (smartSin(t*d) / smartSin(d));
				//point.w *= point.w < 0 ? -1.0 : 1.0;
				vtx.push_back(point);
			}
		
	
		}

		create();
	}

	void toBent() {
		for (int i = 0; i < vtx.size(); i++) {
			vtx[i] = transformPointToCurrentSpace(vtx[i]);
		}
	}

	void create() {
		numberOfVerticies = vtx.size();
		glBufferData(GL_ARRAY_BUFFER, numberOfVerticies * sizeof(vec4), &vtx[0], GL_STATIC_DRAW);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);  // attribute array 0 = POSITION
		//glEnableVertexAttribArray(1);  // attribute array 1 = NORMAL
		// glEnableVertexAttribArray(2);  // attribute array 2 = TEXCOORD0
		// attribute array, components/attribute, component type, normalize?, stride, offset
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), 0);
	}

	void Draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_LINE_STRIP, 0, numberOfVerticies);
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
		M = /*RotationMatrix(rotationAngle, rotationAxis) */ TranslateMatrix(translation);
		if (true) Minv = TranslateMatrix(translation * oppositeVector()) /*RotationMatrix(-rotationAngle, rotationAxis)*/;
		else Minv = TranslateMatrix(-translation) /*RotationMatrix(-rotationAngle, rotationAxis)*/;
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
		Shader * hypShader = new HyperbolicShader();

		// Materials
		Material * material0 = new Material;
		material0->kd = vec3(0.5f, 0.1f, 0.1f);
		material0->ks = vec3(0.5, 0.1,  0.1);
		material0->ka = vec3(0.5f, 0.1f, 0.1f);
		material0->shininess = 100;

		Material * material2 = new Material;
		material2->kd = vec3(0.1f, 0.5f, 0.1f);
		material2->ks = vec3(0.1f, 0.5f, 0.1f);
		material2->ka = vec3(0.1f, 0.5f, 0.1f);
		material2->shininess = 30;


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


		//PARAMS
		float lineSize = 3.14f / 2.0f; //level size /2
		int N = 2 / 2; //bouble grid size /2 -1
		int Ny = 6 / 2;	//number of levels /2 -1

		// Textures
		Texture * texture4x8 = new CheckerBoardTexture(4, 8);
		Texture * texturePlane = new CheckerBoardTexture(40, 40);

		// Geometries
		Sphere * qube1 = new Sphere();
		Plane *plane1 = new Plane(0.0f, lineSize*2, lineSize *2);
		VerticalPlane* plane2 = new VerticalPlane(0.0f, lineSize * 2, lineSize * 2);
		Line* lineX = new Line(transformPointToCurrentSpace(-lineSize, 0.0f, 0.0f), transformPointToCurrentSpace(lineSize, 0.0f, 0.0f)); 
		Line* lineY = new Line(transformPointToCurrentSpace(0.0f, -lineSize, 0.0f), transformPointToCurrentSpace(0.0f, lineSize, 0.0f));
		Line* lineZ = new Line(transformPointToCurrentSpace(0.0f, 0.0f, -lineSize), transformPointToCurrentSpace(0.0f, 0.0f, lineSize));

		// //lines
		// Object * line1 = new Object(hypShader, mater1, texture4x8, lineZ);
		// objects.push_back(line1);

		// Object * line2 = new Object(hypShader, mater1, texture4x8, lineY);
		// objects.push_back(line2);

		// Object * line3 = new Object(hypShader, mater1, texture4x8, lineX);
		// objects.push_back(line3);

		
		
		for (int y = -Ny; y<=Ny; y++) {
			//plane
			if (curvature == SPH && y < 0) continue;
			if (curvature == SPH && y > 0) continue;

			int mat = y+Ny < materials.size() ? y+Ny : 0;
			Object * plane = new Object(hypShader, materials[mat], texturePlane, plane1);

			float height = (curvature == HYP) ? y * 0.7 :  y*3.0;
			plane->translation = transformPointToCurrentSpace(0.0f, height, 0.0f);

			objects.push_back(plane);

			if (curvature != SPH) {

				Object * planeV = new Object(hypShader, materials[mat], texturePlane, plane2);

				float width = (curvature == HYP) ? y * 0.7 : y*3.0;

				planeV->translation = transformPointToCurrentSpace(width, 0.0f, 0.0f);
				objects.push_back(planeV);
			
			}
		}
		
		//spheres
		for (int i = -N; i <= N; i++) {
			for (int j = -N; j <= N; j++) {
				Object * sphere = new Object(hypShader, material0, texture4x8, qube1);
				sphere->translation = transformPointToCurrentSpace(i*lineSize / N, 0.0f, j*lineSize / N);
				objects.push_back(sphere);
			}
		}

		


		// Lights
		Light light;
		light.La = vec3(1.5, 1.5, 1.5); light.Le = vec3(3, 3, 3); light.wLightPos = transformPointToCurrentSpace(0, 0, 0);
		lights.push_back(light);
		Light light2;
		light2.La = vec3(1.5, 1.5, 1.5); light2.Le = vec3(3, 3, 3); light2.wLightPos = transformPointToCurrentSpace(0, 1, 0);
		lights.push_back(light2);
		Light light3;
		light3.La = vec3(1.5, 1.5, 1.5); light3.Le = vec3(3, 3,  3); light3.wLightPos = transformPointToCurrentSpace(0, 0, 2);
		lights.push_back(light3);
	}

	void Render() {
		RenderState state;
		state.wEye = camera.position;
		state.V = camera.V();
		state.P = camera.P();
		state.lights = lights;
		for (auto * obj : objects) {
			if (dynamic_cast<HyperbolicShader*>(obj->shader)) {
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
