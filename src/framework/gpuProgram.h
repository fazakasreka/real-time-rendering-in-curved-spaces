#ifndef GPUPROGRAM_H
#define GPUPROGRAM_H

#include <glad/glad.h>
#include <string>
#include "texture.h"


class GPUProgram {
private:
    unsigned int shaderProgramId = 0;
    unsigned int vertexShader = 0, geometryShader = 0, fragmentShader = 0;
    bool waitError = true;

    void getErrorInfo(unsigned int handle);
    bool checkShader(unsigned int shader, std::string message);
    bool checkLinking(unsigned int program);
    int getLocation(const std::string& name);

public:
    GPUProgram(bool _waitError = true);
    GPUProgram(const GPUProgram& program);
    void operator=(const GPUProgram& program);
    unsigned int getId() { return shaderProgramId; }

    bool create(const char* const vertexShaderSource,
                const char* const fragmentShaderSource, 
                const char* const fragmentShaderOutputName,
                const char* const geometryShaderSource = nullptr);

    void Use();
    void setUniform(int i, const std::string& name);
    void setUniform(float f, const std::string& name);
    void setUniform(const vec2& v, const std::string& name);
    void setUniform(const vec3& v, const std::string& name);
    void setUniform(const vec4& v, const std::string& name);
    void setUniform(const mat4& mat, const std::string& name);
    void setUniform(const Texture& texture, const std::string& samplerName, unsigned int textureUnit = 0);

    ~GPUProgram();
};

#endif // GPUPROGRAM_H