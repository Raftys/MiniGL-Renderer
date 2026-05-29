#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"

class Item {
public:
    std::vector<Mesh> meshes;
    std::unordered_map<std::string, GLuint> textureCache;

    Item(const std::string& path);
    void Draw(const Shader& shader);

private:
    std::string directory;

    void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
    
    // Helpers
    glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from);
    GLuint loadTextureFromFile(const std::string& path);
    GLuint loadTextureFromAssimp(const aiScene* scene, const aiString& texPath);
};