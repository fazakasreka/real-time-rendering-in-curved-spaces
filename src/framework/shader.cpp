#include "shader.h"
#include <fstream>
#include <sstream>

std::string readFile(const char* filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    
    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::stringstream sstr;
    sstr << fileStream.rdbuf();
    content = sstr.str();
    fileStream.close();
    return content;
}

void Shader::setUniformMaterial(const Material& material, const std::string& name) {
    setUniform(material.kd, name + ".kd");
    setUniform(material.ks, name + ".ks");
    setUniform(material.ka, name + ".ka");
    setUniform(material.shininess, name + ".shininess");
}

void Shader::setUniformLight(const Light& light, const std::string& name) {
    setUniform(light.La, name + ".La");
    setUniform(light.Le, name + ".Le");
    setUniform(light.wLightPos, name + ".wLightPos");
}

void Shader::setUniformMaterial(const Material* material, const std::string& name) {
    static Material defaultMaterial;
    const Material* currentMaterial = (material == NULL) ? &defaultMaterial : material;
    setUniform(currentMaterial->kd, name + ".kd");
    setUniform(currentMaterial->ks, name + ".ks");
    setUniform(currentMaterial->ka, name + ".ka");
    setUniform(currentMaterial->shininess, name + ".shininess");
    setUniform(currentMaterial->emission, name + ".emission");
}

void Shader::createShaderFromFiles(const char* vertPath, const char* fragPath, const char* outputName) {
    std::string vertString = readFile(vertPath);
    std::string fragString = readFile(fragPath);
    
    if (vertString.empty() || fragString.empty()) {
        std::cerr << "Error: Shader file(s) not found or empty" << std::endl;
        return;
    }
    
    create(vertString.c_str(), fragString.c_str(), outputName);
}
