#include "shader.h"

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
