# OpenGL GLB Renderer

A lightweight OpenGL renderer built in C++ with support for:

* GLB / GLTF model loading
* Skeletal animation support
* Texture loading
* Vertex colors
* Basic lighting
* FPS-style camera movement
* Assimp model importing
* OpenGL shader abstraction

This project demonstrates a modern OpenGL rendering pipeline using GLFW, GLAD, GLM, Assimp, and stb_image.

---

# Features

* Modern OpenGL (3.3 Core)
* Custom Shader Class
* GLB / GLTF Importing
* Embedded Texture Support
* Skeletal Bone Animation
* Vertex Color Rendering
* Basic Directional Lighting
* Free Camera Controls
* Depth Testing
* Modular Mesh / Item System

---

# Preview

Supports rendering of:

* Terrain
* Animated models
* Vertex-colored meshes
* Textured assets

Example loaded models:

* `ground_grass_v3.glb`
* `elephant_v4.glb`
* `test_color.glb`

---

# Controls

| Key   | Action        |
| ----- | ------------- |
| W     | Move Forward  |
| S     | Move Backward |
| A     | Move Left     |
| D     | Move Right    |
| Mouse | Look Around   |

---

# Dependencies

The project uses the following libraries:

| Library   | Purpose                   |
| --------- | ------------------------- |
| GLFW      | Window creation and input |
| GLAD      | OpenGL function loader    |
| GLM       | Mathematics library       |
| Assimp    | 3D model importing        |
| stb_image | Texture/image loading     |
| OpenGL    | Rendering API             |

---

# Installation

## 1. Clone Repository

```bash
git clone https://github.com/yourname/opengl-glb-renderer.git
cd opengl-glb-renderer
```

---

## 2. Install Dependencies

Make sure you have:

* OpenGL 3.3+
* C++17 compiler
* GLFW
* GLAD
* Assimp
* GLM

---

## 3. Build

Example using g++:

```bash
g++ main.cpp -o app ^
-lglfw ^
-lassimp ^
-lopengl32
```

Linux example:

```bash
g++ main.cpp -o app \
-lglfw \
-lGL \
-lassimp \
-ldl
```

---

# Project Structure

```plaintext
/project
│
├── main.cpp
├── shader.hpp
├── mesh.hpp
├── item.hpp
│
├── ui/
│   ├── ground_grass_v3.glb
│   ├── elephant_v4.glb
│   └── test_color.glb
│
└── LoadUI/
    └── stb_image.h
```

---

# Rendering Pipeline

The renderer currently supports:

1. Model loading through Assimp
2. Vertex processing in GLSL shaders
3. Texture loading through stb_image
4. Directional lighting
5. Camera view/projection matrices
6. Skeletal bone transforms

---

# Shader Features

## Vertex Shader

* Position transforms
* Normal transforms
* Bone matrix blending
* UV support
* Vertex color support

## Fragment Shader

* Texture sampling
* Vertex color fallback
* Basic diffuse lighting

---

# Future Improvements

* PBR Rendering
* Animation playback system
* Shadow mapping
* ECS architecture
* Multiple light types
* Animation blending
* UI system
* Instanced rendering
* Post-processing effects

---

# Credits

## Libraries

### GLFW

Window/context creation and input handling.

https://www.glfw.org/

### GLAD

OpenGL loader generator.

https://glad.dav1d.de/

### GLM

OpenGL Mathematics library.

https://github.com/g-truc/glm

### Assimp

Open Asset Import Library.

https://github.com/assimp/assimp

### stb_image

Single-header image loading library by Sean Barrett.

https://github.com/nothings/stb

---

# License

This project is open-source and available under the MIT License.

---

# Author

Created by YOUR_NAME
