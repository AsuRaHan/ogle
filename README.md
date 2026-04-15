# OGLE3D v0.3

`OGLE3D` is a modern 3D engine/editor written in C++17 for Windows.  It uses Win32 for windowing, OpenGL 4.x for rendering, EnTT‑based ECS for world representation, Dear ImGui for the editor UI, Duktape for JavaScript scripting, and Bullet Physics for rigid‑body simulation.  The project focuses on a clean modular architecture with separate managers for input, rendering, physics, scripting and configuration.

The project already includes:
- a Win32 window with an OpenGL context
- a manager‑based application architecture
- an EnTT‑based ECS world with procedural test scene generation
- a flexible material system with support for multiple textures per object
- camera controls and input handling via a dedicated InputManager
- JSON world serialization (save/load) and configuration files
- editor UI windows: world view, hierarchy, inspector, content browser
- JavaScript runtime scripting through Duktape
- physics simulation with support for static, dynamic, and kinematic rigid bodies (Box, Sphere, Capsule)
- data structures for skeletal animation (clips, tracks, keyframes)

## Current Status

This is an active work-in-progress engine foundation. The project now has a usable in-engine workflow for building a small scene, browsing assets, assigning textures, selecting objects in the viewport, controlling simulation playback, and saving/loading world state. Many systems are still intentionally simple, but the codebase is already practical for experiments and editor iteration.

## Implemented So Far

### Application Architecture
- `App` is now a coordinator instead of a god object
- logic is split into dedicated managers
- config, world, rendering, input, editor, scripting and physics are separated

### Input
- old input classes were adapted to the current project
- `InputManager` receives Win32 messages and updates `InputController`
- `InputActionsManager` configures actions and camera controls
- working controls:
  - `W A S D`
  - `Q / E`
  - `Shift`
  - right mouse button look
  - `Esc` to close the app
- negative axis values and focus-loss input issues were fixed

### Rendering and Windowing
- OpenGL setup and frame rendering are moved into `RenderManager`
- frame timing is moved into `TimeManager`
- camera logic is moved into `CameraManager`
- DPI/viewport resize issues were fixed
- ECS world objects are rendered from world data
- textured materials are supported
- diffuse textures are loaded from disk
- selected objects are highlighted in the viewport
- object `enabled` and `visible` flags affect rendering

### ECS World
- `WorldManager` owns the active world
- `World` uses `EnTT`
- `WorldObject` is a lightweight handle over entities
- currently used components include:
  - `WorldObjectComponent`: Basic state (enabled, visible).
  - `NameComponent`: Entity name.
  - `TransformComponent`: Position, rotation, scale.
  - `ModelComponent`: Link to a 3D model resource.
  - `PrimitiveComponent`: Describes how the geometry was created (e.g., Cube, Sphere, from file).
  - `MaterialComponent`: Defines the object's appearance, can hold multiple textures.
  - `ShaderComponent`: Specifies which shader program to use for rendering.
  - `LightComponent`: Defines a light source (Directional or Point).
  - `PhysicsBodyComponent`: Describes a physical body for simulation (type, shape, mass, etc.).
  - `ScriptComponent`: Attaches a JavaScript file to an entity.
  - `AnimationComponent`: Manages animation state (playing, looping, current time).
  - `SkeletonComponent`: Holds skeletal data for animated models (work in progress).
- default test world exists
- world save/load to JSON exists
- procedural geometry is serialized for save/load
- material texture paths are serialized with world data

### Procedural Objects
- cubes can be created without loading files
- the default world is built procedurally
- the world can be extended from C++ and from scripts
- cubes can use textures

### Assets and Models
- models are loaded through `Assimp`
- diffuse texture paths can be imported from model materials
- a basic material system exists
- a texture cache exists
- the editor includes a content browser rooted at the configured assets folder

### Scripting
- `Duktape` is integrated
- `ScriptManager` loads `.js` files
- `onStart()` and `onUpdate(dt)` are supported
- scripts can spawn and manipulate world objects
- scripts can assign textures to entities
- expanded JavaScript API for creating lights, handling input, and responding to physics collisions (see `docs/js.md`)

### UI and Editor
- `Dear ImGui` is integrated
- the old debug/demo windows are hidden by default
- `Editor` is a separate layer
- the editor can be enabled/disabled
- object picking by mouse click is implemented
- the editor uses separate windows for:
  - `World`
  - `Hierarchy`
  - `Inspector`
  - `Content Browser`
- hierarchy shows scene objects and supports quick add/delete actions
- inspector edits:
  - transform
  - object state
  - texture assignment
  - simple physics settings
- content browser uses the configured assets root folder
- content browser supports drag-and-drop:
  - model file -> world window to spawn entity
  - texture file -> inspector to assign texture
- world window supports load/save/clear/default-world actions
- simulation control supports:
  - play
  - pause
  - step
- script, physics, and world updates can be paused without stopping rendering or input

### Physics
- `Bullet Physics` is integrated
- `PhysicsManager` owns the Bullet world
- supports static, dynamic, and kinematic rigid bodies with Box, Sphere, and Capsule collision shapes
- Bullet bodies sync back into ECS transforms
- default scene creates static and dynamic physics objects
- collision events are dispatched and can be handled in scripts
- editor can add, update, or remove simple box physics settings for selected objects

### Config and File System
- `FileSystem` was added for path and text-file operations
- `AppConfig` and `ConfigManager` were added
- config is loaded from disk or created with defaults
- window settings, editor state, world path, assets path, and script startup behavior are configurable
- if the configured world file is missing, the engine creates a default world and saves it

## Project Layout

```text
src/
  config/
    AppConfig.h
    ConfigManager.h/.cpp

  core/
    FileSystem.h/.cpp

  editor/
    Editor.h/.cpp

  input/
    InputController.h/.cpp
    InputAction.h/.cpp
    AxisBinding.h/.cpp
    InputTypes.h

  managers/
    CameraManager.h/.cpp
    ImGuiManager.h/.cpp
    InputActionsManager.h/.cpp
    InputManager.h/.cpp
    PhysicsManager.h/.cpp
    RenderManager.h/.cpp
    ScriptManager.h/.cpp
    TimeManager.h/.cpp
    WorldManager.h/.cpp

  models/
    BaseModel.h/.cpp
    MeshBuffer.h/.cpp
    ModelEntity.h/.cpp

  render/
    Material.h/.cpp
    Texture2D.h/.cpp

  opengl/
    OpenGLInitializer.h/.cpp
    OpenGLRenderer.h/.cpp
    Camera.h/.cpp
    ShaderManager.h/.cpp

  ui/
    IWindow.h
    Win32Window.h/.cpp

  world/
    World.h/.cpp
    WorldComponents.h
    WorldObject.h/.cpp

  App.h/.cpp
  main.cpp
```

## Dependencies

Dependencies are pulled through `CMake FetchContent`.

- `GLM`
- `Assimp`
- `OpenMesh`
- `EnTT`
- `nlohmann/json`
- `Duktape` with dukglue-based C++ bindings
- `Dear ImGui`
- `Bullet Physics`
- `Windows Imaging Component (WIC)` for texture loading
- Project documentation is available in `docs/`

## Documentation

The project documentation is stored in `docs/`.

- `docs/README.md` — documentation index
- `docs/js.md` — current JavaScript scripting API documentation

## Build

### Requirements
- Windows
- Visual Studio / MSVC
- CMake

### Quick Build

```powershell
.\build.bat
```

Release build:

```powershell
.\build.bat Release
```

### Manual Build

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Run

After building, the executable is placed in `bin/`.

```powershell
.\bin\OGLE3D.exe
```

## Disk Files

Default project paths:

- config: `config/app_config.json`
- world: `data/worlds/default_world.json`
- assets: `assets/`
- scripts: `scripts/*.js`
- log: `log.txt`

If the config file is missing, it is created automatically.

If the world file is missing, the default test world is generated and saved automatically.

## Example Config

```json
{
  "window": {
    "title": "Main window",
    "width": 900,
    "height": 600
  },
  "editor": {
    "enabled": true
  },
  "world": {
    "path": "data/worlds/default_world.json",
    "loadOnStartup": true,
    "saveDefaultWorldIfMissing": true
  },
  "assets": {
    "path": "assets"
  },
  "scripts": {
    "runStartupScript": false,
    "startupScriptPath": "scripts/test_world.js"
  }
}
```

## Approximate Project API

This is not a full generated API reference. It is a practical overview of the main classes and methods already usable in code.

### `App`

Top-level application coordinator.

Main getters:
- `GetCameraManager()`
- `GetEditor()`
- `GetConfigManager()`
- `GetWorldManager()`

### `CameraManager`

Controls the main camera.

Main methods:
- `GetCamera()`
- `SetPosition(...)`
- `SetRotation(...)`
- `LookAt(...)`
- `MoveForward(...)`
- `MoveRight(...)`
- `MoveUp(...)`
- `SetPerspective(...)`
- `SetAspectRatio(...)`
- `SetMode(...)`

Example:

```cpp
app.GetCameraManager().SetPosition({0.0f, 3.0f, 10.0f});
app.GetCameraManager().LookAt({0.0f, 0.0f, 0.0f});
```

### `WorldManager`

Owns and manipulates the active world.

Main methods:
- `GetActiveWorld()`
- `CreateDefaultWorld()`
- `CreateWorldObject(...)`
- `GetWorldObject(entity)`
- `FindEntityByName(...)`
// `CreateCube` has been deprecated in favour of `CreatePrimitive` with `PrimitiveType::Cube`.  The helper is kept only for backward compatibility.
// Use the following pattern instead:
//     worldManager.CreatePrimitive("Cube", OGLE::PrimitiveType::Cube, position, scale, texturePath);
- `CreateModelFromFile(...)`
- `AddModel(...)`
- `ClearWorld()`
- `SetEntityPosition(...)`
- `SetEntityRotation(...)`
- `SetEntityScale(...)`
- `SetEntityDiffuseTexture(...)`
- `SaveActiveWorld(path)`
- `LoadActiveWorld(path)`

Example:

```cpp
// Example using CreatePrimitive with Cube type:
auto entity = app.GetWorldManager().CreatePrimitive(
  "Box",
  OGLE::PrimitiveType::Cube,
  {0.0f, 1.0f, 0.0f},
  {1.0f, 1.0f, 1.0f});

app.GetWorldManager().SetEntityPosition(entity, {2.0f, 1.0f, 0.0f});
```

### `WorldObject`

Lightweight world-entity handle.

Main methods:
- `IsValid()`
- `GetEntity()`
- `GetKind()`
- `GetName()`
- `SetName(...)`
- `GetTransform()`
- `SetTransform(...)`
- `GetModel()`

Example:

```cpp
auto object = app.GetWorldManager().GetWorldObject(entity);
object.SetTransform(
    {0.0f, 2.0f, 0.0f},
    {0.0f, 45.0f, 0.0f},
    {1.0f, 1.0f, 1.0f});
```

### `PhysicsManager`

Basic physics integration through Bullet.

Main methods:
- `Initialize(worldManager)`
- `SetGravity(...)`
- `AddBoxBody(entity, halfExtents, bodyType, mass)`
- `RemoveBody(entity)`
- `Clear()`
- `Update(deltaTime)`
- `GetBodyCount()`

Example:

```cpp
m_physicsManager.AddBoxBody(
    entity,
    {0.5f, 0.5f, 0.5f},
    OGLE::PhysicsBodyType::Dynamic,
    1.0f);
```

### `ScriptManager`

Runs JavaScript through `Duktape`.

Main methods:
- `Initialize(worldManager)`
- `ExecuteFile(path)`
- `Update(deltaTime)`

The scripting API is exposed via the global `ogle` object in JavaScript. It allows for creating and manipulating entities, controlling physics, logging, and handling input. For a detailed API reference, see `docs/js.md`.

Example API functions:
- `ogle.world.createCube(...)`
- `ogle.world.createPointLight(...)`
- `ogle.entity.setPosition(...)`
- `ogle.physics.addBox(...)`
- `ogle.input.isKeyDown(...)`

Example script:

```js
var cubeId = null;

function onStart() {
    cubeId = ogle.world.createCube("ScriptedCube", [0, 1, 0], [1, 1, 1]);
    ogle.log("Cube created with ID: " + cubeId);
}

function onUpdate(dt) {
    if (cubeId !== null && ogle.entity.exists(cubeId)) {
        var rot = ogle.entity.getRotation(cubeId);
        rot[1] += 45.0 * dt; // Rotate around Y axis
        ogle.entity.setRotation(cubeId, rot);
    }
}
```

### `Editor`

Editor layer built on top of ImGui.

Main methods:
- `Initialize()`
- `SetEnabled(bool)`
- `Toggle()`
- `IsEnabled()`
- `BuildUi(...)`

Current features:
- world window
- hierarchy window
- inspector window
- content browser window
- selected object tracking
- click selection in the scene
- viewport highlight for the selected object
- transform editing
- object visibility and enabled editing
- texture assignment
- simple physics editing
- drag-and-drop from content browser into the world and inspector
- simulation play/pause/step

### `ConfigManager`

Loads and saves application configuration.

Main methods:
- `Load()`
- `Save()`
- `LoadOrCreateDefault()`
- `GetConfig()`
- `GetConfigPath()`

### `FileSystem`

Minimal file system layer.

Main methods:
- `Exists(path)`
- `EnsureDirectory(path)`
- `EnsureParentDirectory(path)`
- `ReadTextFile(path, content)`
- `WriteTextFile(path, content)`
- `GetWorkingDirectory()`
- `GetExecutableDirectory()`
- `ResolvePath(path)`

## Reasonable Next Steps

- transform gizmo in the viewport
- better object outlines / selection rendering
- scene parenting and true hierarchy relationships
- asset preview thumbnails
- spawn dragged models under mouse position in the scene
- script reload tools in the editor
- more robust material system
- skinned meshes and animation runtime
- prefab system
- broader resource file system

## Changes in v0.2

- added textured rendering and basic material support
- added texture loading through WIC
- switched model loading to an Assimp-based runtime path for mesh and diffuse texture import
- added world serialization for procedural geometry and material texture paths
- expanded the editor into separate `World`, `Hierarchy`, `Inspector`, and `Content Browser` windows
- added a configurable assets root folder in config and editor UI
- added content browser drag-and-drop for models and textures
- added selected-object viewport highlighting
- added scene object create/delete tools in hierarchy and world windows
- added simulation `Play`, `Pause`, and `Step` controls
- made simulation updates controllable without stopping rendering or input
- connected visibility and enabled object flags to rendering

## Note

This README is intended to describe the actual current codebase, not an aspirational design. It should be updated together with the architecture.
