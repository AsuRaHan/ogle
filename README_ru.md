# OGLE20 v0.2

`OGLE20` — это экспериментальный 3D-движок/редактор на C++17 с Win32, OpenGL, ECS-управлением миром через `EnTT`, редакторским UI на `Dear ImGui`, скриптами через `Duktape`, текстурированным рендерингом, браузером ассетов и базовой физикой на `Bullet`.

Сейчас в проекте уже есть:
- Win32-окно и OpenGL-контекст
- архитектура приложения на менеджерах
- ECS-мир и world-объекты
- процедурная тестовая сцена
- текстурированный рендер мешей
- перемещение камеры и система инпута
- сохранение/загрузка мира в JSON
- сохранение/загрузка конфигурации на диск
- окна редактора для мира, иерархии, инспектора и браузера контента
- startup/runtime-скрипты на JavaScript
- базовая физика твёрдых тел

## Текущее состояние

Это всё ещё активно развивающаяся основа движка, но проект уже дошёл до состояния, в котором можно собирать небольшую сцену прямо в редакторе, просматривать ассеты, назначать текстуры, выбирать объекты во viewport, управлять симуляцией и сохранять/загружать состояние мира. Многие системы пока намеренно простые, но кодовая база уже пригодна для экспериментов и дальнейшего развития инструментов.

## Что уже реализовано

### Архитектура приложения
- `App` больше не является god-object
- логика разделена по отдельным менеджерам
- `config`, `world`, `rendering`, `input`, `editor`, `scripting` и `physics` разделены по подсистемам

### Ввод
- старые input-классы адаптированы под текущий проект
- `InputManager` получает Win32-сообщения и обновляет `InputController`
- `InputActionsManager` настраивает actions и управление камерой
- рабочее управление:
  - `W A S D`
  - `Q / E`
  - `Shift`
  - вращение камеры правой кнопкой мыши
  - `Esc` для закрытия приложения
- исправлены проблемы с отрицательными значениями осей и потерей фокуса

### Рендеринг и окно
- OpenGL-инициализация и рендер кадра вынесены в `RenderManager`
- тайминг кадра вынесен в `TimeManager`
- логика камеры вынесена в `CameraManager`
- исправлены проблемы DPI/resize/viewports
- world-объекты из ECS рендерятся напрямую из данных мира
- поддерживаются текстурированные материалы
- diffuse-текстуры загружаются с диска
- выбранный объект подсвечивается во viewport
- флаги `enabled` и `visible` реально влияют на рендер

### ECS-мир
- `WorldManager` владеет активным миром
- `World` построен на `EnTT`
- `WorldObject` — это лёгкая обёртка над entity
- сейчас используются компоненты:
  - `WorldObjectComponent`
  - `NameComponent`
  - `TransformComponent`
  - `ModelComponent`
  - `PhysicsBodyComponent`
- есть дефолтный тестовый мир
- есть сохранение/загрузка мира в JSON
- процедурная геометрия сериализуется и переживает save/load
- пути к текстурам материалов тоже сериализуются вместе с миром

### Процедурные объекты
- кубы можно создавать без загрузки файлов
- дефолтный мир строится процедурно
- мир можно расширять из C++ и из скриптов
- процедурные кубы могут использовать текстуры

### Ассеты и модели
- модели загружаются через `Assimp`
- diffuse texture path может импортироваться из материала модели
- есть базовая система материалов
- есть кэш текстур
- в редактор добавлен content browser с корнем в настраиваемой папке ассетов

### Скрипты
- интегрирован `Duktape`
- `ScriptManager` загружает `.js`-файлы
- поддерживаются `onStart()` и `onUpdate(dt)`
- скрипты могут создавать и изменять world-объекты
- скрипты могут назначать текстуры объектам

### UI и редактор
- интегрирован `Dear ImGui`
- старые debug/demo-окна по умолчанию скрыты
- `Editor` выделен как отдельный слой
- редактор можно включать/выключать
- реализован выбор объектов мышкой во viewport
- редактор использует отдельные окна:
  - `World`
  - `Hierarchy`
  - `Inspector`
  - `Content Browser`
- `Hierarchy` показывает объекты сцены и поддерживает быстрые действия добавления/удаления
- `Inspector` позволяет редактировать:
  - transform
  - состояние объекта
  - назначение текстуры
  - простые physics-настройки
- `Content Browser` работает от корня, заданного в конфиге
- `Content Browser` поддерживает drag-and-drop:
  - файл модели -> в окно `World`, чтобы создать объект
  - файл текстуры -> в `Inspector`, чтобы назначить текстуру
- окно `World` поддерживает load/save/clear/default world
- управление симуляцией поддерживает:
  - play
  - pause
  - step
- обновления `script`, `physics` и `world` можно ставить на паузу, не останавливая рендер и ввод

### Физика
- интегрирован `Bullet Physics`
- `PhysicsManager` владеет Bullet-world
- поддерживаются базовые box rigid bodies
- Bullet-тела синхронизируют transform обратно в ECS
- дефолтная сцена создаёт статические и динамические physics-объекты
- редактор умеет добавлять, обновлять и удалять простые physics-настройки для выбранного объекта

### Конфиг и файловая система
- добавлен `FileSystem` для путей и текстовых файлов
- добавлены `AppConfig` и `ConfigManager`
- конфиг загружается с диска или создаётся с дефолтами
- настраиваются параметры окна, состояние редактора, путь к миру, путь к ассетам и startup-скрипт
- если world-файл отсутствует, движок создаёт дефолтный мир и сохраняет его

## Структура проекта

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

## Зависимости

Зависимости подтягиваются через `CMake FetchContent`.

- `GLM`
- `Assimp`
- `OpenMesh`
- `EnTT`
- `nlohmann/json`
- `Duktape`
- `Dear ImGui`
- `Bullet Physics`
- `Windows Imaging Component (WIC)` для загрузки текстур

## Сборка

### Требования
- Windows
- Visual Studio / MSVC
- CMake

### Быстрая сборка

```powershell
.\build.bat
```

Release-сборка:

```powershell
.\build.bat Release
```

### Ручная сборка

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Запуск

После сборки исполняемый файл находится в `bin/`.

```powershell
.\bin\OGLE3D.exe
```

## Файлы на диске

Дефолтные пути проекта:

- config: `config/app_config.json`
- world: `data/worlds/default_world.json`
- assets: `assets/`
- scripts: `scripts/*.js`
- log: `log.txt`

Если config-файл отсутствует, он будет создан автоматически.

Если world-файл отсутствует, будет создан и сохранён дефолтный тестовый мир.

## Пример конфига

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

## Приблизительное API проекта

Это не полный автогенерированный API-референс. Это практичный обзор основных классов и методов, которыми уже можно пользоваться.

### `App`

Главный координатор приложения.

Основные getters:
- `GetCameraManager()`
- `GetEditor()`
- `GetConfigManager()`
- `GetWorldManager()`

### `CameraManager`

Управляет главной камерой.

Основные методы:
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

Пример:

```cpp
app.GetCameraManager().SetPosition({0.0f, 3.0f, 10.0f});
app.GetCameraManager().LookAt({0.0f, 0.0f, 0.0f});
```

### `WorldManager`

Владеет активным миром и управляет им.

Основные методы:
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
- `SetEntityDiffuseTexture(...)`
- `SaveActiveWorld(path)`
- `LoadActiveWorld(path)`

Пример:

```cpp
auto entity = app.GetWorldManager().CreateCube(
    "Box",
    {0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f});

app.GetWorldManager().SetEntityPosition(entity, {2.0f, 1.0f, 0.0f});
```

### `WorldObject`

Лёгкая обёртка над world-entity.

Основные методы:
- `IsValid()`
- `GetEntity()`
- `GetKind()`
- `GetName()`
- `SetName(...)`
- `GetTransform()`
- `SetTransform(...)`
- `GetModel()`

Пример:

```cpp
auto object = app.GetWorldManager().GetWorldObject(entity);
object.SetTransform(
    {0.0f, 2.0f, 0.0f},
    {0.0f, 45.0f, 0.0f},
    {1.0f, 1.0f, 1.0f});
```

### `PhysicsManager`

Базовая физическая интеграция через Bullet.

Основные методы:
- `Initialize(worldManager)`
- `SetGravity(...)`
- `AddBoxBody(entity, halfExtents, bodyType, mass)`
- `RemoveBody(entity)`
- `Clear()`
- `Update(deltaTime)`
- `GetBodyCount()`

Пример:

```cpp
m_physicsManager.AddBoxBody(
    entity,
    {0.5f, 0.5f, 0.5f},
    OGLE::PhysicsBodyType::Dynamic,
    1.0f);
```

### `ScriptManager`

Запускает JavaScript через `Duktape`.

Основные методы:
- `Initialize(worldManager)`
- `ExecuteFile(path)`
- `Update(deltaTime)`

Сейчас в JS доступны функции:
- `log(message)`
- `clearWorld()`
- `spawnCube(name, x, y, z, sx, sy, sz)`
- `setPosition(entityId, x, y, z)`
- `setRotation(entityId, x, y, z)`
- `setScale(entityId, x, y, z)`
- `setTexture(entityId, path)`
- `entityExists(entityId)`

Пример скрипта:

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

Редакторский слой поверх ImGui.

Основные методы:
- `Initialize()`
- `SetEnabled(bool)`
- `Toggle()`
- `IsEnabled()`
- `BuildUi(...)`

Текущие возможности:
- окно мира
- окно иерархии
- окно инспектора
- окно браузера контента
- отслеживание выбранного объекта
- выбор объекта кликом во viewport
- подсветка выбранного объекта во viewport
- редактирование transform
- редактирование `visible` и `enabled`
- назначение текстур
- редактирование простых physics-параметров
- drag-and-drop из content browser в мир и в inspector
- управление симуляцией: play/pause/step

### `ConfigManager`

Загружает и сохраняет конфигурацию приложения.

Основные методы:
- `Load()`
- `Save()`
- `LoadOrCreateDefault()`
- `GetConfig()`
- `GetConfigPath()`

### `FileSystem`

Минимальный слой файловой системы.

Основные методы:
- `Exists(path)`
- `EnsureDirectory(path)`
- `EnsureParentDirectory(path)`
- `ReadTextFile(path, content)`
- `WriteTextFile(path, content)`
- `GetWorkingDirectory()`
- `GetExecutableDirectory()`
- `ResolvePath(path)`

## Разумные следующие шаги

- gizmo-манипулятор прямо во viewport
- более выразительный outline / selection rendering
- parent/child-связи и настоящая иерархия сцены
- превью ассетов в content browser
- спавн модели под курсором в сцене при drag-and-drop
- инструменты перезагрузки скриптов из редактора
- более богатая material-система
- skinned meshes и runtime-анимация
- prefab-система
- более широкая resource/file-system подсистема

## Изменения в v0.2

- добавлен текстурированный рендер и базовая поддержка материалов
- добавлена загрузка текстур через WIC
- загрузка моделей переведена на runtime-путь через Assimp для мешей и diffuse-текстур
- добавлена сериализация процедурной геометрии и путей материалов в world JSON
- редактор расширен до отдельных окон `World`, `Hierarchy`, `Inspector` и `Content Browser`
- добавлен настраиваемый корень ассетов в конфиг и UI редактора
- добавлен drag-and-drop моделей и текстур из content browser
- добавлена подсветка выбранного объекта во viewport
- добавлены инструменты создания/удаления объектов в окнах мира и иерархии
- добавлены элементы управления симуляцией `Play`, `Pause` и `Step`
- обновления симуляции теперь можно останавливать, не трогая рендер и ввод
- флаги `visible` и `enabled` подключены к рендеру

## Примечание

Этот README описывает текущее состояние кода, а не желаемую архитектуру в будущем. Его стоит обновлять вместе с развитием проекта.
