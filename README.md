# Xi Engine

<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/c73da522-c35c-419c-bb57-5753cc143d53" />

A modern, lightweight 3D game engine built with C++20 and OpenGL 4.5. 

[Download](https://github.com/essam-tobgi-dev/Xi-Engine/releases/tag/xi-engine)

## Overview

Xi Engine is a cross-platform game engine designed for real-time 3D applications. It features a modular architecture with an Entity Component System (ECS), physically-based rendering (PBR), and an integrated editor interface.

## Features

### Core Systems
- **Application Framework** - Window management, input handling, and game loop
- **Entity Component System** - Flexible, data-oriented architecture for game objects
- **Resource Management** - Centralized loading and caching of assets

### Rendering
- **OpenGL 4.5 Renderer** - Modern graphics pipeline with shader-based rendering
- **Physically-Based Rendering** - PBR materials with metallic-roughness workflow
- **Multiple Light Types** - Directional, point, and spot lights
- **Render Queue** - Automatic sorting for opaque and transparent objects
- **Framebuffer Support** - Off-screen rendering for editor viewports

### Editor
- **ImGui Integration** - Immediate mode GUI for editor tools
- **Scene Hierarchy** - Visual tree view of all entities
- **Inspector Panel** - Component editing interface
- **Console** - Runtime logging and debugging
- **Scene Viewport** - Real-time 3D preview with camera controls

### Physics
- **Collision Detection** - Box and sphere colliders
- **Rigid Body Dynamics** - Mass, velocity, and force-based simulation
- **Fixed Timestep** - Deterministic physics updates

### Audio
- **MiniAudio Backend** - Cross-platform audio playback
- **Audio Sources** - Positional audio components
- **Audio Clips** - Sound asset management

## Demo

<img width="757" height="450" alt="xi" src="https://github.com/user-attachments/assets/0d81d29e-26f7-46ed-951b-a36124aab375" />

https://github.com/user-attachments/assets/879ba0b3-0a62-4773-9b56-8ccb670f8a19

## Project Structure

```
Xi Engine/
├── Engine/
│   ├── Core/           # Application, Window, Input, Time, Logging
│   ├── ECS/            # Entity, Component, System, World
│   │   └── Components/ # Transform, MeshRenderer, Light, etc.
│   ├── Renderer/       # Shader, Mesh, Material, Camera, Framebuffer
│   ├── Physics/        # PhysicsWorld, Collision, Colliders
│   ├── Audio/          # AudioEngine, AudioClip
│   ├── Editor/         # EditorUI, SceneHierarchy, Inspector, Console
│   └── Resources/      # ResourceManager, SceneSerializer
├── Game/               # Game-specific application code
├── vendor/             # Third-party dependencies
└── Shaders/            # GLSL shader files
```

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [GLFW](https://www.glfw.org/) | 3.x | Window and input management |
| [GLEW](http://glew.sourceforge.net/) | 2.x | OpenGL extension loading |
| [GLM](https://github.com/g-truc/glm) | 0.9.9+ | Mathematics library |
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.89+ | Editor interface |
| [stb](https://github.com/nothings/stb) | - | Image loading |
| [MiniAudio](https://miniaud.io/) | - | Audio playback |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.x | Scene serialization |

## Building

### Requirements
- Visual Studio 2022 (v145 toolset)
- Windows 10 SDK
- C++20 compatible compiler

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/essam-tobgi-dev/Xi-Engine.git
   cd Xi-Engine
   ```

2. Open `Xi Engine.sln` in Visual Studio 2022

3. Select the desired configuration:
   - **Debug x64** - Development with debugging symbols
   - **Release x64** - Optimized build

4. Build the solution (F7 or Build > Build Solution)

5. Run the application (F5)

## Controls

### Editor Camera
- **Right-click + Drag** - Enable camera control
- **W/A/S/D** - Move forward/left/backward/right
- **Q/E** - Move down/up
- **Shift** - Increase movement speed
- **Mouse** - Look around

### Application
- **Escape** - Quit application

## Usage Example

```cpp
#include "Engine/Core/Application.h"
#include "Engine/ECS/World.h"
#include "Engine/ECS/Components/Transform.h"
#include "Engine/ECS/Components/MeshRenderer.h"

class MyGame : public Xi::Application {
public:
    void OnInit() override {
        Xi::World& world = GetWorld();
        Xi::Renderer& renderer = GetRenderer();

        // Create an entity
        Xi::Entity cube = world.CreateEntity("Cube");

        // Add components
        auto& transform = world.AddComponent<Xi::Transform>(cube);
        transform.position = glm::vec3(0.0f, 1.0f, 0.0f);

        auto& meshRenderer = world.AddComponent<Xi::MeshRenderer>(cube);
        meshRenderer.mesh = Xi::Primitives::CreateCube();
        meshRenderer.material = CreateMaterial(renderer);
    }
};
```

## Architecture

### Entity Component System

Xi Engine uses a component-based architecture where:
- **Entities** are unique identifiers (IDs)
- **Components** are pure data structures
- **Systems** operate on entities with specific component combinations

### Rendering Pipeline

1. Scene objects submit render commands to the queue
2. Commands are sorted (front-to-back for opaque, back-to-front for transparent)
3. Materials bind shaders and set uniforms
4. Meshes are drawn with the appropriate render state

### Editor Integration

The editor renders the scene to an off-screen framebuffer, which is then displayed in an ImGui window. This allows for viewport manipulation without affecting the final output.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.txt) file for details.

## Acknowledgments

- [Learn OpenGL](https://learnopengl.com/) - Graphics programming tutorials
- [Game Engine Architecture](https://www.gameenginebook.com/) - Engine design reference
- [EnTT](https://github.com/skypjack/entt) - ECS design inspiration
