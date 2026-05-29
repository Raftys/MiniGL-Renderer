#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define STB_IMAGE_IMPLEMENTATION
#include "LoadUI/stb_image.h"

#include "shader.hpp"
#include "mesh.hpp"
#include "item.hpp"

// --------------------------------------------------
// Helper Logic
// --------------------------------------------------

// Convert Assimp matrix format into GLM matrix format
glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
    glm::mat4 to;

    // Copy each matrix component manually
    to[0][0] = from.a1; to[1][0] = from.a2;
    to[2][0] = from.a3; to[3][0] = from.a4;

    to[0][1] = from.b1; to[1][1] = from.b2;
    to[2][1] = from.b3; to[3][1] = from.b4;

    to[0][2] = from.c1; to[1][2] = from.c2;
    to[2][2] = from.c3; to[3][2] = from.c4;

    to[0][3] = from.d1; to[1][3] = from.d2;
    to[2][3] = from.d3; to[3][3] = from.d4;

    return to;
}

// Load a texture from disk using stb_image
GLuint loadTextureFromFile(const std::string& path) {
    int w, h, channels;

    // Flip textures vertically because OpenGL expects origin at bottom-left
    stbi_set_flip_vertically_on_load(true);

    // Load image data
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);

    // Return 0 if loading failed
    if (!data) return 0;

    // Determine OpenGL texture format
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    GLuint tex;

    // Generate and bind texture object
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Upload texture to GPU
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        (GLint)format,
        w,
        h,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    // Generate mipmaps for scaling
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free CPU image memory
    stbi_image_free(data);

    return tex;
}

// Load texture either from embedded GLB texture or external file
GLuint loadTextureFromAssimp(
    const aiScene* scene,
    const aiString& texPath,
    const std::string& modelDir
) {
    std::string path = texPath.C_Str();

    // Embedded texture begins with '*'
    if (!path.empty() && path[0] == '*') {

        // Get embedded texture index
        int index = std::stoi(path.substr(1));

        const aiTexture* tex = scene->mTextures[index];

        int w, h, channels;

        // Load texture directly from memory
        unsigned char* data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(tex->pcData),
            (tex->mHeight == 0)
                ? tex->mWidth
                : tex->mWidth * tex->mHeight * 4,
            &w,
            &h,
            &channels,
            0
        );

        if (!data) return 0;

        GLuint glTex;

        glGenTextures(1, &glTex);
        glBindTexture(GL_TEXTURE_2D, glTex);

        GLenum fmt = (channels == 4) ? GL_RGBA : GL_RGB;

        // Upload texture data to GPU
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            fmt,
            w,
            h,
            0,
            fmt,
            GL_UNSIGNED_BYTE,
            data
        );

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        return glTex;
    }

    // Otherwise load external texture file
    return loadTextureFromFile(modelDir + "/" + path);
}

// --------------------------------------------------
// Camera & Mouse Controls
// --------------------------------------------------

struct Camera {

    // Camera vectors
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Rotation angles
    float Yaw;
    float Pitch;

    // Constructor
    Camera(glm::vec3 pos)
        : Position(pos),
          Front(0,0,-1),
          WorldUp(0,1,0),
          Yaw(-90.0f),
          Pitch(0.0f)
    {
        update();
    }

    // Generate camera view matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Handle WASD movement
    void ProcessKeyboard(GLFWwindow* w, float dt) {

        float speed = 2.5f * dt;

        if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
            Position += Front * speed;

        if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
            Position -= Front * speed;

        if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
            Position -= Right * speed;

        if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
            Position += Right * speed;
    }

    // Recalculate camera direction vectors
    void update() {

        glm::vec3 f;

        // Convert yaw/pitch angles into direction vector
        f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        f.y = sin(glm::radians(Pitch));
        f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

        Front = glm::normalize(f);

        // Calculate right and up vectors
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

// Mouse movement callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    static float lastX = 640;
    static float lastY = 360;

    static bool first = true;

    // Prevent sudden camera jump on first frame
    if(first) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        first = false;
    }

    // Mouse movement offsets
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    // Retrieve camera pointer from GLFW
    auto cam = (Camera*)glfwGetWindowUserPointer(window);

    // Apply mouse sensitivity
    cam->Yaw   += xoffset * 0.1f;
    cam->Pitch += yoffset * 0.1f;

    // Prevent camera flipping
    if(cam->Pitch > 89.0f)  cam->Pitch = 89.0f;
    if(cam->Pitch < -89.0f) cam->Pitch = -89.0f;

    // Recalculate vectors
    cam->update();
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int main() {

    // Initialize GLFW
    glfwInit();

    // Create OpenGL window
    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "Shader Class Integration",
        nullptr,
        nullptr
    );

    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // --------------------------------------------------
    // Vertex Shader
    // --------------------------------------------------

    const char* vs = R"(

        #version 330 core

        // Vertex attributes
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNorm;
        layout(location = 2) in vec2 aUV;
        layout(location = 3) in vec3 aColor;
        layout(location = 4) in ivec4 aBoneIDs;
        layout(location = 5) in vec4 aWeights;

        // Data sent to fragment shader
        out vec2 UV;
        out vec3 Norm;
        out vec3 vColor;

        // Uniform matrices
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProj;

        // Bone animation matrices
        uniform mat4 finalBonesMatrices[100];

        void main() {

            // Final skinning transform
            mat4 boneTransform = mat4(0.0);

            // Blend up to 4 bone transforms
            for (int i = 0; i < 4; i++) {

                if (aBoneIDs[i] >= 0)
                    boneTransform +=
                        finalBonesMatrices[aBoneIDs[i]]
                        * aWeights[i];
            }

            UV = aUV;
            vColor = aColor;

            // Transform normals correctly
            Norm = mat3(transpose(inverse(uModel))) * aNorm;

            // Final clip-space position
            gl_Position =
                uProj *
                uView *
                uModel *
                vec4(aPos, 1.0);
        }

    )";

    // --------------------------------------------------
    // Fragment Shader
    // --------------------------------------------------

    const char* fs = R"(

        #version 330 core

        out vec4 FragColor;

        in vec2 UV;
        in vec3 Norm;
        in vec3 vColor;

        uniform sampler2D uTex;
        uniform bool uHasTex;

        void main() {

            // Use texture if available
            // Otherwise use vertex color
            vec3 baseColor =
                uHasTex
                ? texture(uTex, UV).rgb
                : vColor;

            // Simple directional lighting
            float d = max(
                dot(
                    normalize(Norm),
                    normalize(vec3(3, 10, 3))
                ),
                0.3
            );

            FragColor = vec4(baseColor * d, 1.0);
        }

    )";

    // Compile shader program
    Shader shader(vs, fs);

    // Load 3D models
    Item myItem("ui/ground_grass_v3.glb");
    Item myItem2("ui/elephant_v4.glb");
    Item myItem3("ui/test_color.glb");

    // Create camera
    Camera camera(glm::vec3(0, 2, 5));

    // Connect camera to mouse callback
    glfwSetWindowUserPointer(window, &camera);

    glfwSetCursorPosCallback(window, mouse_callback);

    // Hide and lock cursor
    glfwSetInputMode(
        window,
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    // --------------------------------------------------
    // Main Render Loop
    // --------------------------------------------------

    while (!glfwWindowShouldClose(window)) {

        static float lastFrame = 0;

        // Delta time for smooth movement
        float dt = (float)glfwGetTime() - lastFrame;
        lastFrame += dt;

        // Handle keyboard movement
        camera.ProcessKeyboard(window, dt);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        shader.Use();

        // Create projection matrix
        glm::mat4 proj = glm::perspective(
            glm::radians(60.0f),
            1280.0f / 720.0f,
            0.1f,
            100.0f
        );

        // Send camera matrices to shader
        shader.SetMat4("uView", camera.GetViewMatrix());
        shader.SetMat4("uProj", proj);

        // Draw all models
        myItem.Draw(shader);
        myItem2.Draw(shader);
        myItem3.Draw(shader);

        // Present frame
        glfwSwapBuffers(window);

        // Process input/events
        glfwPollEvents();
    }

    // Cleanup GLFW
    glfwTerminate();

    return 0;
}