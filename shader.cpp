#include "shader.hpp"
#include <iostream>

Shader::Shader(const char* vs_src, const char* fs_src) {
    auto compile = [](GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        
        int success;
        char infoLog[512];
        glGetShaderiv(s, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(s, 512, NULL, infoLog);
            std::cerr << "Error::Shader::Compilation_Failed\n" << infoLog << std::endl;
        }
        return s;
    };

    GLuint v = compile(GL_VERTEX_SHADER, vs_src);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs_src);

    ID = glCreateProgram();
    glAttachShader(ID, v);
    glAttachShader(ID, f);
    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "Error::Shader::Program::Linking_Failed\n" << infoLog << std::endl;
    }

    glDeleteShader(v);
    glDeleteShader(f);
}

Shader::~Shader() {
   
}

void Shader::Use() const { glUseProgram(ID); }

void Shader::SetMat4(const std::string& name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
}

void Shader::SetMat4Array(const std::string& name, const std::vector<glm::mat4>& mats) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), (GLsizei)mats.size(), GL_FALSE, glm::value_ptr(mats[0]));
}