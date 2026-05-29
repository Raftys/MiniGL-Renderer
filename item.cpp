#include "item.hpp"
#include <iostream>
#include "LoadUI/stb_image.h"

Item::Item(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << "\n";
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

void Item::Draw(const Shader& shader) {
    for (auto& mesh : meshes) {
        mesh.Draw(shader);
    }
}

void Item::processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform) {
    // Combine parent transform with local node transform
    glm::mat4 nodeTransform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, nodeTransform));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, nodeTransform);
    }
}

Mesh Item::processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Normal matrix for correct lighting (handles scaling/rotation)
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        
        // 1. Load and Transform Position
        glm::vec4 localPos = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        v.Position = glm::vec3(localPos);

        // 2. Transform Normal
        if (mesh->HasNormals()) {
            v.Normal = glm::normalize(normalMatrix * glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        }

        // 3. UVs
        v.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
        
        // 4. Vertex Colors (Fix for vertex paint)
        v.Color = mesh->HasVertexColors(0) ? 
                  glm::vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b) : 
                  glm::vec3(1.0f);

        // 5. Initialize bone info
        v.BoneIDs = glm::ivec4(-1);
        v.Weights = glm::vec4(0.0f);

        vertices.push_back(v);
    }

    // 6. Handle Bones/Weights
    if (mesh->HasBones()) {
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            aiBone* bone = mesh->mBones[i];
            for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                unsigned int vertexID = bone->mWeights[j].mVertexId;
                float weight = bone->mWeights[j].mWeight;

                if (weight <= 0.0001f) continue; // FIX: Zero out tiny weights

                for (int k = 0; k < 4; k++) {
                    if (vertices[vertexID].BoneIDs[k] == -1) {
                        vertices[vertexID].BoneIDs[k] = i;
                        vertices[vertexID].Weights[k] = weight;
                        break;
                    }
                }
            }
        }
    }

    // 7. Load Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // 8. Texture Loading
    GLuint tex = 0;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiString str;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS) {
            std::string texKey = str.C_Str();
            if (textureCache.count(texKey)) {
                tex = textureCache[texKey];
            } else {
                tex = loadTextureFromAssimp(scene, str);
                textureCache[texKey] = tex;
            }
        }
    }

    return Mesh(vertices, indices, tex, transform);
}

// Helpers Implementation
glm::mat4 Item::aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

GLuint Item::loadTextureFromFile(const std::string& path) {
    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) return 0;
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    GLuint tex; glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}

GLuint Item::loadTextureFromAssimp(const aiScene* scene, const aiString& texPath) {
    std::string path = texPath.C_Str();
    if (!path.empty() && path[0] == '*') {
        int index = std::stoi(path.substr(1));
        const aiTexture* tex = scene->mTextures[index];
        int w, h, channels;
        unsigned char* data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(tex->pcData), 
                              (tex->mHeight == 0) ? tex->mWidth : tex->mWidth * tex->mHeight * 4, &w, &h, &channels, 0);
        if (!data) return 0;
        GLuint glTex; glGenTextures(1, &glTex);
        glBindTexture(GL_TEXTURE_2D, glTex);
        GLenum fmt = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return glTex;
    }
    return loadTextureFromFile(directory + "/" + path);
}