#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h> 
#else
#include <glad/glad.h>
#endif

#include <string>
#include <vector>
#include "frameworkMath.h"

class Texture {
    std::vector<vec4> load(std::string pathname, bool transparent, int& width, int& height);

public:
    unsigned int textureId;

    Texture();
    Texture(std::string pathname, bool transparent = false);
    Texture(int width, int height, const std::vector<vec4>& image, int sampling = GL_LINEAR);
    Texture(const Texture& texture);
    void operator=(const Texture& texture);
    void create(std::string pathname, bool transparent = false);
    void create(int width, int height, const std::vector<vec4>& image, int sampling = GL_LINEAR);
    ~Texture();
}; 

#endif // TEXTURE_H