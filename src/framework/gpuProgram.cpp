#include "gpuProgram.h"
#include <stdio.h>

void GPUProgram::getErrorInfo(unsigned int handle) {
	int logLen, written;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		std::string log(logLen, '\0');
		glGetShaderInfoLog(handle, logLen, &written, &log[0]);
		printf("Shader log:\n%s", log.c_str());
		if (waitError) getchar();
	}
}

bool GPUProgram::checkShader(unsigned int shader, std::string message) {
	int OK;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
	if (!OK) {
		printf("%s!\n", message.c_str());
		getErrorInfo(shader);
		return false;
	}
	return true;
}

bool GPUProgram::checkLinking(unsigned int program) {
	int OK;
	glGetProgramiv(program, GL_LINK_STATUS, &OK);
	if (!OK) {
		printf("Failed to link shader program!\n");
		int logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			std::string log(logLen, '\0');
			glGetProgramInfoLog(program, logLen, nullptr, &log[0]);
			printf("Program log:\n%s", log.c_str());
		}
		return false;
	}
	return true;
}

int GPUProgram::getLocation(const std::string& name) {
	int location = glGetUniformLocation(shaderProgramId, name.c_str());
	if (location < 0) printf("uniform %s cannot be set\n", name.c_str());
	return location;
}

GPUProgram::GPUProgram(bool _waitError) {
	shaderProgramId = 0;
	waitError = _waitError;
}

GPUProgram::GPUProgram(const GPUProgram& program) {
	if (program.shaderProgramId > 0) printf("\nError: GPU program is not copied on GPU!!!\n");
}

void GPUProgram::operator=(const GPUProgram& program) {
	if (program.shaderProgramId > 0) printf("\nError: GPU program is not copied on GPU!!!\n");
}

bool GPUProgram::create(const char* const vertexShaderSource,
					   const char* const fragmentShaderSource,
					   const char* const geometryShaderSource) 
{
	// Create vertex shader from string
	if (vertexShader == 0) vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (!vertexShader) {
		printf("Error in vertex shader creation\n");
		exit(1);
	}
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	if (!checkShader(vertexShader, "Vertex shader error")) return false;

	// Create geometry shader from string if given
#ifdef __EMSCRIPTEN__
	// GLES2 does not support geometry shaders
	if (geometryShaderSource != nullptr) {
		printf("Geometry shaders are not supported in WebGL\n");
		return false;
	}
#else
	if (geometryShaderSource != nullptr) {
		if (geometryShader == 0) geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		if (!geometryShader) {
			printf("Error in geometry shader creation\n");
			exit(1);
		}
		glShaderSource(geometryShader, 1, (const GLchar**)&geometryShaderSource, NULL);
		glCompileShader(geometryShader);
		if (!checkShader(geometryShader, "Geometry shader error")) return false;
	}
#endif

	// Create fragment shader from string
	if (fragmentShader == 0) fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fragmentShader) {
		printf("Error in fragment shader creation\n");
		exit(1);
	}

	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	if (!checkShader(fragmentShader, "Fragment shader error")) return false;

	// Attach shaders to program
	shaderProgramId = glCreateProgram();
	if (!shaderProgramId) {
		printf("Error in shader program creation\n");
		exit(1);
	}
	glAttachShader(shaderProgramId, vertexShader);
	glAttachShader(shaderProgramId, fragmentShader);
#ifndef __EMSCRIPTEN__
	if (geometryShader > 0) glAttachShader(shaderProgramId, geometryShader);
	glBindFragDataLocation(shaderProgramId, 0, "fragColor");
#endif

	// program packaging
	glLinkProgram(shaderProgramId);
	if (!checkLinking(shaderProgramId)) return false;

	// make this program run
	glUseProgram(shaderProgramId);
	return true;
}

void GPUProgram::Use() {
	glUseProgram(shaderProgramId);
}

void GPUProgram::setUniform(int i, const std::string& name) {
	int location = getLocation(name);
	if (location >= 0) glUniform1i(location, i);
}

void GPUProgram::setUniform(float f, const std::string& name) {
	int location = getLocation(name);
	if (location >= 0) glUniform1f(location, f);
}

void GPUProgram::setUniform(const vec2& v, const std::string& name) {
	int location = getLocation(name);
	if (location >= 0) glUniform2fv(location, 1, &v.x);
}

void GPUProgram::setUniform(const vec3& v, const std::string& name) {
	int location = getLocation(name);
	if (location >= 0) glUniform3fv(location, 1, &v.x);
}

void GPUProgram::setUniform(const vec4& v, const std::string& name) {
	int location = getLocation(name);
	if (location >= 0) glUniform4fv(location, 1, &v.x);
}

void GPUProgram::setUniform(const mat4& mat, const std::string& name) {
	int location = getLocation(name);
	if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, mat);
}

void GPUProgram::setUniform(const Texture& texture, const std::string& samplerName, unsigned int textureUnit) {
	int location = getLocation(samplerName);
	if (location >= 0) {
		glUniform1i(location, textureUnit);
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture.textureId);
	}
}

GPUProgram::~GPUProgram() {
	if (shaderProgramId > 0) glDeleteProgram(shaderProgramId);
}
