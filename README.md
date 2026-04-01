# OGLE20

`OGLE20` is an experimental 3D engine/editor project on C++17 with Win32, OpenGL, ECS world management through `EnTT`, editor UI through `Dear ImGui`, scripting through `Duktape`, and basic physics through `Bullet`.

The project already includes:
- a Win32 window and OpenGL context
- manager-based application architecture
- ECS world and world objects
- procedural test scene generation
- camera movement and input actions
- JSON world save/load
- config file load/save on disk
- editor layer with object picking and inspector
- JavaScript startup/runtime scripting
- basic rigid body physics

## Current Status

This is an active work-in-progress engine foundation. The architecture is already split into subsystems and usable for experiments, but many systems are still intentionally minimal.

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

### ECS World
- `WorldManager` owns the active world
- `World` uses `EnTT`
- `WorldObject` is a lightweight handle over entities
- currently used components include:
  - `WorldObjectComponent`
  - `NameComponent`
  - `TransformComponent`
  - `ModelComponent`
  - `PhysicsBodyComponent`
- default test world exists
- world save/load to JSON exists

### Procedural Objects
- cubes can be created without loading files
- the default world is built procedurally
- the world can be extended from C++ and from scripts

### Scripting
- `Duktape` is integrated
- `ScriptManager` loads `.js` files
- `onStart()` and `onUpdate(dt)` are supported
- scripts can spawn and manipulate world objects

### UI and Editor
- `Dear ImGui` is integrated
- `ImGuiManager` renders the general debug UI
- `Editor` is a separate layer
- the editor can be enabled/disabled
- object picking by mouse click is implemented
- `Inspector` shows the currently selected object
- transform editing for the selected entity is available

### Physics
- `Bullet Physics` is integrated
- `PhysicsManager` owns the Bullet world
- basic box rigid bodies are supported
- Bullet bodies sync back into ECS transforms
- default scene creates static and dynamic physics objects

### Config and File System
- `FileSystem` was added for path and text-file operations
- `AppConfig` and `ConfigManager` were added
- config is loaded from disk or created with defaults
- window settings, editor state, world path and script startup behavior are configurable
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
- `Duktape`
- `Dear ImGui`
- `Bullet Physics`

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
- `CreateCube(...)`
- `CreateModelFromFile(...)`
- `AddModel(...)`
- `ClearWorld()`
- `SetEntityPosition(...)`
- `SetEntityRotation(...)`
- `SetEntityScale(...)`
- `SaveActiveWorld(path)`
- `LoadActiveWorld(path)`

Example:

```cpp
auto entity = app.GetWorldManager().CreateCube(
    "Box",
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

Currently exposed JS functions:
- `log(message)`
- `clearWorld()`
- `spawnCube(name, x, y, z, sx, sy, sz)`
- `setPosition(entityId, x, y, z)`
- `setRotation(entityId, x, y, z)`
- `setScale(entityId, x, y, z)`
- `entityExists(entityId)`

Example script:

```js
var cube = -1;

function onStart() {
    cube = spawnCube("ScriptedCube", 0, 1, 0, 1, 1, 1);
}

function onUpdate(dt) {
    if (entityExists(cube)) {
        setRotation(cube, 0, dt * 45.0, 0);
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
- editor window
- selected object tracking
- click selection in the scene
- inspector window
- transform editing for the selected entity

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

- editor main menu bar
- hierarchy panel
- richer inspector for ECS components
- save/load world from editor UI
- editor hotkeys
- physics body creation from UI and scripts
- selected object highlight
- gizmo manipulation
- prefab system
- broader resource file system

## Note

This README is intended to describe the actual current codebase, not an aspirational design. It should be updated together with the architecture.
