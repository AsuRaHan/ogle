# COPILOT INSTRUCTIONS — OGLE20 Game Engine

## Who Are You

You are an AI coding assistant working on the **OGLE20** game engine — a custom 3D engine written in **C++ (17)** with **OpenGL 3.3 Core Profile**. The engine includes a visual editor built with **ImGui**. Your job is to implement features, fix bugs, and refactor code as instructed by the human developer.

**You are NOT writing a standalone app or a web project.** Everything you do is inside this existing C++ codebase.

---

## Project Location and Build

```
Path:      E:\my_proj\ogle
Build:     PS E:\my_proj\ogle> .\build.bat Release
Test:      Run the executable that build.bat produces
```

**Always build after making changes.** If the build fails, read the error carefully and fix it. Do not ask the user to fix compilation errors — that is your job.

---

## Tech Stack

| Technology | Version / Details | Where Used |
|---|---|---|
| C++ | C++17 (MSVC on Windows) | Everything |
| OpenGL | 3.3 Core Profile | All rendering |
| GLSL | `#version 330 core` | All shaders |
| GLFW | (check CMakeLists.txt) is not used — Win32 raw window | Window management |
| ImGui | Latest | Editor UI |
| ImGuizmo | Latest | Transform gizmos (translate/rotate/scale) |
| EnTT | Latest | ECS (Entity Component System) |
| nlohmann/json | Latest | World serialization, config, materials |
| GLM | Latest | Math (vec3, mat4, etc.) |
| stb_image | May or may not be present | Image loading fallback |
| Windows WIC | COM-based | Texture loading (primary) |

**Platform:** Windows only. Uses Win32 API for window creation, message loop, and input. Uses WIC (Windows Imaging Component) for texture loading.

---

## Project Structure

```
E:\my_proj\ogle\
├── assets/
│   ├── shaders/              ← GLSL shader files (.vs, .fs)
│   ├── worlds/               ← JSON world save files
│   ├── scripts/              ← JavaScript scripts for ScriptManager
│   └── textures/             ← Texture files (PNG, etc.)
├── src/
│   ├── core/                 ← Core utilities
│   │   ├── Layer.h / LayerStack.h
│   │   ├── FileSystem.h / FileSystem.cpp
│   │   ├── Logger.h
│   │   ├── EventBus.h        ← Event Bus (type-based pub/sub)
│   │   └── Events.h          ← Event type definitions
│   ├── editor/               ← Editor UI panels
│   │   ├── Editor.h / Editor.cpp
│   │   ├── EditorState.h
│   │   ├── EditorCreationPanel.h / .cpp
│   │   ├── EditorHierarchyPanel.h / .cpp
│   │   ├── EditorInspectorPanel.h / .cpp
│   │   ├── EditorAnimationPanel.h / .cpp
│   │   ├── EditorContentBrowserPanel.h / .cpp
│   │   ├── EditorProceduralTexturePanel.h / .cpp
│   │   └── EditorAssetHelpers.h / .cpp
│   ├── input/                ← Input system
│   │   ├── InputController.h / .cpp   (singleton, namespace OGLE)
│   │   ├── InputManager.h / .cpp      (older, used by App)
│   │   ├── InputAction.h
│   │   ├── InputTypes.h
│   │   └── AxisBinding.h
│   ├── managers/             ← Engine subsystems
│   │   ├── CameraManager.h / .cpp
│   │   ├── ConfigManager.h / .cpp
│   │   ├── ImGuiManager.h / .cpp
│   │   ├── InputActionsManager.h / .cpp
│   │   ├── InputManager.h / .cpp
│   │   ├── PhysicsManager.h / .cpp
│   │   ├── RenderManager.h / .cpp
│   │   ├── ScriptManager.h / .cpp
│   │   ├── TimeManager.h / .cpp
│   │   └── WorldManager.h / .cpp
│   ├── opengl/               ← OpenGL wrappers
│   │   ├── Camera.h / .cpp
│   │   ├── GLFunctions.h     ← GL function loader (glew-style)
│   │   ├── ShaderManager.h / .cpp
│   │   └── OpenGLRenderer.h / .cpp
│   ├── render/               ← Rendering resources
│   │   ├── Material.h / .cpp
│   │   ├── MaterialLibrary.h / .cpp
│   │   ├── AnimationLibrary.h / .cpp
│   │   ├── Texture2D.h / .cpp
│   │   ├── TextureManager.h       (singleton, header-only)
│   │   ├── ProceduralTexture.h / .cpp
│   │   └── ModelEntity.h / .cpp
│   ├── ui/                   ← Window abstraction
│   │   ├── IWindow.h
│   │   └── Win32Window.h / .cpp
│   ├── world/                ← ECS world
│   │   ├── World.h / .cpp
│   │   ├── WorldComponents.h
│   │   └── WorldObject.h / .cpp
│   ├── config/               ← Configuration
│   │   └── ConfigManager.h / .cpp
│   ├── physics/              ← Physics system
│   │   └── ...
│   ├── App.h / App.cpp       ← Main application class
│   └── main.cpp              ← Entry point
├── build.bat                 ← Build script
├── CMakeLists.txt            ← CMake configuration
└── *.sln / *.vcxproj         ← Visual Studio files (may exist)
```

---

## Code Conventions

### Namespaces

There is **one namespace** in this project. All engine classes and types live in `OGLE`:

```cpp
namespace OGLE {
    // Core classes:
    class Camera;
    class InputController;
    class Texture2D;
    class Material;
    class World;
    class WorldObject;
    class ModelEntity;

    // Entity type:
    using Entity = entt::entity;

    // Components:
    struct TransformComponent;
    struct WorldObjectComponent;
    struct NameComponent;
    struct ModelComponent;
    struct MaterialComponent;
    struct LightComponent;
    struct PhysicsBodyComponent;
    struct ShaderComponent;
    struct SkeletonComponent;
    struct AnimationComponent;
    struct ScriptComponent;
    struct PrimitiveComponent;
}
```

**IMPORTANT:** Always use the `OGLE::` prefix for engine types. Do NOT use lowercase `ogle::`.

### File Naming

- Header files: `PascalCase.h` (e.g. `Material.h`, `WorldManager.h`)
- Source files: `PascalCase.cpp` (e.g. `Material.cpp`, `WorldManager.cpp`)
- Shader files: `lowercase.vs`, `lowercase.fs` (e.g. `default.vs`, `shadow.fs`)
- GLSL uniforms: `uCamelCase` prefix `u` (e.g. `uBaseColor`, `uMVP`)
- GLSL attributes: `aCamelCase` prefix `a` (e.g. `aPosition`, `aNormal`)
- GLSL varyings: `vCamelCase` prefix `v` (e.g. `vWorldPosition`, `vTexCoord`)
- GLSL varyings from vertex shader: `vCamelCase` prefix `v`

### Include Style

```cpp
// Local includes use relative paths with the src/ root:
#include "render/Material.h"
#include "core/FileSystem.h"
#include "opengl/Camera.h"
#include "world/WorldComponents.h"

// Third-party includes use angle brackets:
#include <glm/glm.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>
```

### C++ Patterns

- **Prefer member initialization list** in constructors
- **Use `std::unique_ptr`** for owned heap objects
- **Use `std::shared_ptr`** for shared resources (textures)
- **Use references** for non-owning access to managers
- **Use `entt::null`** to check for invalid entities (NOT nullptr)
- **Use `std::array<char, N>`** for fixed-size text buffers in ImGui (NOT std::string)
- **Always use `std::strncpy`** with buffer size for copying into char arrays

### Singleton Pattern

The project uses multiple singletons:

```cpp
// Access patterns:
App::Get()                      // Global app instance
ogle::InputController::Get()    // Input controller
ShaderManager::GetGlobalInstance()  // Shader manager (may exist)
OGLE::MaterialLibrary::Instance()   // Material library
OGLE::AnimationLibrary::Instance()  // Animation library
TextureManager::Get()           // Texture manager (header-only singleton)
```

---

## Architecture

### Main Loop (App::Run)

```
1. PeekMessage / DispatchMessage (Win32 message pump)
2. m_timeManager.Tick()           → deltaTime
3. m_inputManager.Update()        → process input
4. for layer in layerStack:
     layer->OnUpdate(deltaTime)   → game logic, physics, scripts
5. m_imguiManager.BeginFrame()    → start ImGui
6. for layer in layerStack:
     layer->OnImGuiRender()       → editor UI
7. m_renderManager.RenderFrame()  → OpenGL rendering + ImGui render
```

### Manager System

`App` is the central hub. It owns all managers. Managers are accessed through `App::Get().GetXxxManager()`:

```
App (singleton)
├── Window (IWindow / Win32Window)
├── ConfigManager
├── CameraManager
├── ImGuiManager
├── InputManager
├── InputActionsManager
├── PhysicsManager
├── RenderManager
│   └── OpenGLRenderer (owns ShaderManager)
├── ScriptManager
├── TimeManager
├── WorldManager
│   └── World (owns EnTT registry)
├── Editor
│   ├── EditorState
│   ├── EditorCreationPanel
│   ├── EditorHierarchyPanel
│   ├── EditorInspectorPanel
│   ├── EditorAnimationPanel
│   ├── EditorContentBrowserPanel
│   └── EditorProceduralTexturePanel
└── LayerStack
```

### Layer System

The engine uses a layer-based architecture for the update and render loop:

```cpp
class Layer {
    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnImGuiRender() {}
};
```

`LayerStack` stores `Layer*` pointers. Layers are pushed in order and iterated from first to last.

Currently there are two layers:
1. `MainApplicationLayer` (defined inside App.cpp) — runs game logic, physics, scripts
2. `ExampleLayer` — appears to be a test/example layer

### ECS (Entity Component System)

Uses **EnTT** library. Components are plain structs in `world/WorldComponents.h`:

```cpp
// Key components:
OGLE::TransformComponent  { position, rotation, scale }
OGLE::WorldObjectComponent { kind, enabled, visible }
OGLE::NameComponent       { value (string) }
OGLE::ModelComponent      { model (shared_ptr<ModelEntity>) }
OGLE::MaterialComponent   { material (Material) }
OGLE::LightComponent      { type, color, intensity, range, castShadows, primary }
OGLE::PhysicsBodyComponent { type, shape, mass, halfExtents, radius, height, simulate, isTrigger }
OGLE::ShaderComponent     { programName }
OGLE::ScriptComponent     { scriptPath, enabled, autoStart }
OGLE::SkeletonComponent   { sourcePath, boneCount, enabled }
OGLE::AnimationComponent  { currentClip, playing, loop, currentTime, playbackSpeed, enabled }
OGLE::PrimitiveComponent  { type, sourcePath }
```

World access:
```cpp
auto& world = worldManager.GetActiveWorld();
auto& registry = world.GetRegistry();
auto view = registry.view<OGLE::TransformComponent, OGLE::LightComponent>();
for (auto entity : view) {
    auto& transform = view.get<OGLE::TransformComponent>(entity);
    auto& light = view.get<OGLE::LightComponent>(entity);
}
```

### Event Bus

A type-based publish/subscribe system. Located in `core/EventBus.h` (header-only).

```cpp
// Subscribe:
m_eventBus.Subscribe<MyEvent>([](const MyEvent& e) { ... });

// Dispatch:
m_eventBus.Dispatch(MyEvent{ ... });

// Available events (in core/Events.h):
OGLE::WindowResizeEvent    { width, height }
OGLE::CollisionEvent       { entityA, entityB }
OGLE::EditorLoadWorldEvent { path }
OGLE::EditorSaveWorldEvent { path }
// ... and more editor events
```

### Shader System

Shaders are loaded from `assets/shaders/` as text files and compiled at runtime via `ShaderManager`:

```cpp
m_shaderManager.LoadShaderSource("assets/shaders/default.vs");
m_shaderManager.loadVertexShader("default_vs", source);
m_shaderManager.loadFragmentShader("default_fs", source);
m_shaderManager.linkProgram("default", "default_vs", "default_fs");
m_shaderManager.useProgram("default");
```

Shader programs are identified by string name ("default", "shadow_depth", "grid", "debug_line", etc.).

---

## Current Feature State

### Rendering
| Feature | Status |
|---|---|
| PBR-like lighting (Blinn-Phong) | ✅ Done |
| Shadow mapping (directional) | ✅ Done |
| PCF shadow smoothing (3x3) | ✅ Done |
| Multiple point lights (up to 4) | ✅ Done |
| Grid + axis gizmo | ✅ Done |
| Debug line shader | ✅ Done (reusable) |

### NOT Yet Implemented
| Feature | Status |
|---|---|
| Post-processing (bloom, tone mapping, FXAA, SSAO) | ❌ |
| Skybox | ❌ |
| Cascaded Shadow Maps (CSM) | ❌ |
| HDR rendering | ❌ |
| Texture compression (DDS/KTX) | ❌ |
| GPU instancing | ❌ |

### Editor
| Feature | Status |
|---|---|
| Entity hierarchy | ✅ Done |
| Inspector (all components) | ✅ Done |
| ImGuizmo (translate/rotate/scale) | ✅ Done |
| World save/load | ✅ Done |
| Content browser | ✅ Done |
| Procedural texture generator | ✅ Done |
| Event-driven buttons | ✅ Done (via EventBus) |
| Undo/Redo | ❌ |
| Multi-select | ❌ |
| Copy/Paste entities | ❌ |
| Debug overlay (wireframe, normals) | ❌ |

### Systems
| Feature | Status |
|---|---|
| Event Bus | ✅ Done |
| Texture Manager (with fallback, cache, stats) | ✅ Done |
| Physics (custom, not a library) | ✅ Done |
| Scripting (JavaScript via embeddable engine) | ✅ Done |
| Skeleton/Animation system | ✅ Done (basic) |
| Asset Manager (centralized) | ❌ |

---

## Rules for Making Changes

### 1. ALWAYS Build After Changes
```
PS E:\my_proj\ogle> .\build.bat Release
```
If the build fails, fix the error yourself. Do not leave the code in a broken state.

### 2. Do NOT Break Existing Functionality
Every change must be additive or a safe refactor. The engine must compile and run after your changes. All existing features (shadows, editor, physics, scripts) must continue working.

### 3. Always Use OGLE:: Namespace
All engine types use the `OGLE::` namespace:
- `OGLE::Camera`, `OGLE::InputController`, `OGLE::Texture2D`, `OGLE::Material`, `OGLE::World`
- `OGLE::Entity`, `OGLE::TransformComponent`, etc.
- Do NOT use lowercase `ogle::`

### 4. Be Careful with EnTT Entities
- Invalid entity = `entt::null` (NOT nullptr)
- Entity type = `OGLE::Entity` which is `typedef entt::entity`
- Always check validity: `worldManager.IsEntityValid(entity)` before accessing components
- Entity IDs are recycled — do not store them long-term without validation

### 5. OpenGL Resource Management
- Always clean up GL resources in destructors (glDeleteTextures, glDeleteBuffers, glDeleteVertexArrays, glDeleteFramebuffers)
- Check for `!= 0` before deleting
- Bind texture unit 0 after rendering (`glActiveTexture(GL_TEXTURE0)`)

### 6. ImGui Best Practices
- Use `std::array<char, N>` for text input buffers, NOT `std::string`
- Use `std::strncpy(buf.data(), str.c_str(), buf.size() - 1)` to copy strings
- Check `ImGui::GetIO().WantCaptureMouse/Keyboard` before processing input
- Use `ImGui::BeginDisabled()` / `ImGui::EndDisabled()` for grayed-out buttons

### 7. Editor Panels Get EditorState& for State, WorldManager& for Data
Editor panels receive `EditorState&` (for UI state) and `WorldManager&` (for reading world data). When implementing new panels, follow this same pattern:
```cpp
void MyPanel::Draw(EditorState& state, WorldManager& worldManager)
```

### 8. For Write Operations — Use Events
Editor panels should NOT call manager methods that modify world state directly. Instead, dispatch events through `state.eventBus`. The event handlers live in `App.cpp`:
```cpp
// ❌ WRONG (direct call):
worldManager.CreatePrimitive(name, type, pos, scale, texPath);

// ✅ CORRECT (event):
state.eventBus->Dispatch(ogle::EditorCreateEntityEvent{ type, name, "", texPath });
```

### 9. Shader Files
- Always start with `#version 330 core`
- Use `#ifdef GL_ES` guards if targeting both desktop and mobile (currently not needed)
- Test that shader compiles by running the engine after changes
- Use `fwidth()` for anti-aliased lines (available in GLSL 330)
- All uniforms must be set every frame (no persistent uniform state)

### 10. File Paths
- Use forward slashes `/` in code (Windows accepts both)
- The engine uses `FileSystem::ResolvePath()` to resolve relative paths
- Asset paths are relative to the project root: `"assets/shaders/default.vs"`
- Use `std::filesystem::path` for path manipulation

---

## Common Pitfalls

1. **Forgetting `#include "Logger.h"`** — LOG_INFO, LOG_WARN, LOG_ERROR won't compile without it.

2. **Using lowercase `ogle::`** — The correct namespace is `OGLE::` (uppercase). Do NOT use `ogle::`.

3. **Using `nullptr` for entities** — EnTT entities are integers, not pointers. Use `entt::null`.

4. **Not checking ImGui WantCapture** — Input will pass through ImGui windows without this check.

5. **GL state leaks** — After rendering special passes (grid, shadows), always restore GL state (blend, depth mask, cull face, active texture).

6. **Modifying components while iterating** — EnTT does NOT allow modifying or deleting components while iterating a view. Collect entities to modify, then modify after the loop.

7. **Forgetting to reset frame state** — Input controller resets pressed/released flags each frame. Do not call `ResetFrameState()` more than once.

8. **Shadow map texture unit** — Shadow map uses texture unit 2 (`GL_TEXTURE2`). Do NOT bind other textures to unit 2 during the main render pass.

---

## When You Receive a Task

1. **Read the task carefully.** Understand what is being asked.
2. **Read the relevant existing files** before making changes. Understand the current code.
3. **Plan your changes** — which files to create/modify, in what order.
4. **Make changes incrementally.** Build after each major change.
5. **Test by running the engine.** If something looks wrong, investigate.
6. **If you encounter an error you cannot fix**, describe what you tried and what went wrong. Do not silently give up.

---

## Development Roadmap (Current Priorities)

These are the features planned for near-term implementation:

1. **Post-processing pipeline** — tone mapping + gamma correction + FXAA
2. **Undo/Redo system** — command pattern for editor operations
3. **Debug rendering** — wireframe AABB, physics shapes, light helpers
4. **Skybox** — cubemap or gradient background
5. **Audio system** — 3D positional audio
6. **Particle system** — GPU particles via compute shader
7. **Asset Manager** — centralized resource loading and management
8. **Prefab system** — save/load entity templates

---

## Useful OpenGL Patterns in This Project

### Setting up a shader program:
```cpp
std::string vsSrc = m_shaderManager.LoadShaderSource("assets/shaders/myshader.vs");
std::string fsSrc = m_shaderManager.LoadShaderSource("assets/shaders/myshader.fs");
m_shaderManager.loadVertexShader("myshader_vs", vsSrc.c_str());
m_shaderManager.loadFragmentShader("myshader_fs", fsSrc.c_str());
m_shaderManager.linkProgram("myshader", "myshader_vs", "myshader_fs");
```

### Getting uniform locations:
```cpp
const GLuint program = m_shaderManager.getProgram("myshader");
const GLint loc = glGetUniformLocation(program, "uMyUniform");
if (loc >= 0) {
    glUniform1f(loc, value);
}
```

### Creating a VAO/VBO for line rendering:
```cpp
GLuint vao, vbo;
glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glBindVertexArray(vao);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset);
glBindVertexArray(0);
```

### Texture loading:
```cpp
// Always go through TextureManager (which replaces Texture2D::LoadShared):
auto texture = TextureManager::Get().Load("assets/textures/wood.png");
// Returns fallback magenta texture if file not found
// Returns default white texture if path is empty
```

---

## Summary

You are working on a **C++17 / OpenGL 3.3 / Win32** game engine with an **ImGui editor** and **EnTT ECS**. The project lives at `E:\my_proj\ogle` and builds with `.\build.bat Release`. Be careful with namespaces, entity types, and OpenGL state. Always build and test after changes. Use the Event Bus for editor actions that modify world state. Follow existing code patterns and conventions.
