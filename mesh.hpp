#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.hpp"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Color;
    glm::ivec4 BoneIDs = glm::ivec4(-1); // Initialized to -1 for bone logic
    glm::vec4 Weights = glm::vec4(0.0f);
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint texture;
    glm::mat4 localTransform;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, GLuint tex, const glm::mat4& transform);
    
    void Draw(const Shader& shader) const;

private:
    GLuint vao, vbo, ebo;
    void setupMesh();
};