#include "geometry.h"

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

Geometry::Geometry() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

Geometry::~Geometry() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Geometry::bindBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}


ParamGeometry::ParamGeometry() : nVtxPerStrip(0), nStrips(0) {}

VertexData ParamGeometry::GenVertexData(float u, float v) {
    VertexData vtxData;
    vtxData.texcoord = vec2(u, v);
    
    Dnum2 X, Y, Z;
    Dnum2 U(u, vec2(1, 0)), V(v, vec2(0, 1));
    eval(U, V, X, Y, Z);
    
    vtxData.position = vec4(X.f, Y.f, Z.f, 1.0f);
    
    vec3 drdU(X.d.x, Y.d.x, Z.d.x);
    vec3 drdV(X.d.y, Y.d.y, Z.d.y);
    vec3 normal = euclideanNormalize(euclideanCross(drdU, drdV));
    vtxData.normal = vec4(normal.x, normal.y, normal.z, 0.0f);
    
    return vtxData;
}

void ParamGeometry::create(int N, int M) {
    nVtxPerStrip = (M + 1) * 2;
    nStrips = N;
    std::vector<VertexData> vtxData;    // vertices on the CPU
    
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

void ParamGeometry::Draw() {
    glBindVertexArray(vao);
    for (unsigned int i = 0; i < nStrips; i++) {
        glDrawArrays(GL_TRIANGLE_STRIP, i * nVtxPerStrip, nVtxPerStrip);
    }
}