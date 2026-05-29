#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

class Shader {
public:
    GLuint ID = 0;

    Shader(const char* vs_src, const char* fs_src);
    ~Shader();

    void Use() const;
    void SetMat4(const std::string& name, const glm::mat4& m) const;
    void SetVec3(const std::string& name, const glm::vec3& v) const;
    void SetInt(const std::string& name, int value) const;
    void SetBool(const std::string& name, bool value) const;
    void SetMat4Array(const std::string& name, const std::vector<glm::mat4>& mats) const;
};