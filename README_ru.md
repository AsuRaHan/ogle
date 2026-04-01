# OGLE20

`OGLE20` — это учебно-практический 3D-движок/редактор на C++17.  
Он работает через `Win32 + OpenGL`, хранит мир через `EnTT`, умеет показывать UI через `Dear ImGui`, запускать JavaScript через `Duktape` и считать простую физику через `Bullet`.

Если говорить совсем просто, сейчас проект уже умеет:
- открыть окно
- создать 3D-мир
- нарисовать объекты
- двигать камерой
- выбирать объект мышкой
- показывать его свойства в редакторе
- сохранять мир в файл
- загружать мир обратно
- запускать скрипты
- применять простую физику

Этот `README_ru.md` написан подробнее и более “по-человечески”, чтобы его можно было читать даже если ты ещё только входишь в тему движков.

## Идея проекта

Цель проекта — собрать свою основу движка, где есть:
- окно приложения
- рендер
- ввод
- мир с объектами
- редакторский интерфейс
- скрипты
- физика
- файлы конфигурации и данные проекта

То есть это уже не просто “рисуем треугольник”, а каркас маленького движка и редактора.

## Что уже сделано

### 1. Окно и OpenGL

Проект создаёт обычное Win32-окно и поднимает OpenGL-контекст.

За это отвечают:
- [IWindow](/e:/my_proj/OGLE20/src/ui/IWindow.h)
- [Win32Window.h](/e:/my_proj/OGLE20/src/ui/Win32Window.h)
- [Win32Window.cpp](/e:/my_proj/OGLE20/src/ui/Win32Window.cpp)
- [OpenGLInitializer.h](/e:/my_proj/OGLE20/src/opengl/OpenGLInitializer.h)
- [OpenGLInitializer.cpp](/e:/my_proj/OGLE20/src/opengl/OpenGLInitializer.cpp)

Что важно:
- исправлена проблема с DPI
- исправлена проблема с resize окна
- теперь картинка корректно подстраивается под размер клиентской области

### 2. Менеджеры

Раньше логика могла лежать в `App`, но сейчас проект уже разделён на отдельные подсистемы.

Сейчас есть такие менеджеры:
- `InputManager`
- `InputActionsManager`
- `TimeManager`
- `RenderManager`
- `CameraManager`
- `WorldManager`
- `ScriptManager`
- `PhysicsManager`
- `ImGuiManager`
- `ConfigManager`

Это удобно, потому что:
- код легче читать
- проще искать баги
- проще добавлять новые системы
- `App` становится координатором, а не “свалкой всего сразу”

### 3. Ввод

Система ввода уже работает в базовом виде.

Что есть сейчас:
- клавиатура
- мышь
- action-ы и axis-ы
- камера управляется через бинды

Клавиши:
- `W / S` — вперёд / назад
- `A / D` — влево / вправо
- `Q / E` — вниз / вверх
- `Shift` — ускорение
- зажатая правая кнопка мыши — обзор мышью
- `Esc` — закрыть приложение

Файлы:
- [InputController.h](/e:/my_proj/OGLE20/src/input/InputController.h)
- [InputController.cpp](/e:/my_proj/OGLE20/src/input/InputController.cpp)
- [InputAction.h](/e:/my_proj/OGLE20/src/input/InputAction.h)
- [InputAction.cpp](/e:/my_proj/OGLE20/src/input/InputAction.cpp)
- [InputManager.cpp](/e:/my_proj/OGLE20/src/managers/InputManager.cpp)
- [InputActionsManager.cpp](/e:/my_proj/OGLE20/src/managers/InputActionsManager.cpp)

Что уже исправлялось:
- отрицательные оси, из-за которых не работали `A` и `S`
- залипание клавиш при потере фокуса

### 4. Камера

Камера вынесена в отдельный менеджер, чтобы её можно было менять из любого места проекта более чисто.

Файлы:
- [CameraManager.h](/e:/my_proj/OGLE20/src/managers/CameraManager.h)
- [CameraManager.cpp](/e:/my_proj/OGLE20/src/managers/CameraManager.cpp)

Что умеет:
- ставить позицию
- менять поворот
- смотреть в точку
- двигаться по локальным осям
- менять проекцию

Пример:

```cpp
app.GetCameraManager().SetPosition({0.0f, 3.0f, 10.0f});
app.GetCameraManager().LookAt({0.0f, 0.0f, 0.0f});
```

### 5. Мир и объекты

Сейчас мир хранится через `EnTT`.  
Это значит, что в проекте уже используется ECS-подход:
- есть сущность (`entity`)
- есть компоненты
- объект мира — это по сути сущность + набор компонентов

Главные файлы:
- [World.h](/e:/my_proj/OGLE20/src/world/World.h)
- [World.cpp](/e:/my_proj/OGLE20/src/world/World.cpp)
- [WorldComponents.h](/e:/my_proj/OGLE20/src/world/WorldComponents.h)
- [WorldObject.h](/e:/my_proj/OGLE20/src/world/WorldObject.h)
- [WorldObject.cpp](/e:/my_proj/OGLE20/src/world/WorldObject.cpp)
- [WorldManager.h](/e:/my_proj/OGLE20/src/managers/WorldManager.h)
- [WorldManager.cpp](/e:/my_proj/OGLE20/src/managers/WorldManager.cpp)

Основные компоненты:
- `WorldObjectComponent`
- `NameComponent`
- `TransformComponent`
- `ModelComponent`
- `PhysicsBodyComponent`

Простыми словами:
- `NameComponent` хранит имя
- `TransformComponent` хранит позицию, поворот и масштаб
- `ModelComponent` хранит модель для рендера
- `PhysicsBodyComponent` хранит данные физического тела

### 6. Процедурные объекты

Сейчас можно создавать хотя бы простые кубы без загрузки из файла.

Это важно, потому что:
- можно быстро строить тестовые сцены
- можно делать объекты прямо из кода
- можно создавать объекты из скриптов

Пример:

```cpp
auto entity = app.GetWorldManager().CreateCube(
    "Box",
    {0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f});
```

### 7. Тестовый мир

В проекте есть тестовый мир по умолчанию.

Если файла мира ещё нет:
- создаётся дефолтная сцена
- она сохраняется на диск

Если файл мира уже есть:
- он загружается

Это уже похоже на нормальную логику движка:
- сначала есть данные проекта
- потом движок их читает
- если данных нет, создаёт стартовый вариант

### 8. Редактор

Редактор сейчас уже существует как отдельный класс, а не просто “пара окон ImGui”.

Файлы:
- [Editor.h](/e:/my_proj/OGLE20/src/editor/Editor.h)
- [Editor.cpp](/e:/my_proj/OGLE20/src/editor/Editor.cpp)

Что умеет сейчас:
- инициализироваться отдельно
- включаться и выключаться
- рисовать своё окно
- хранить выбранный объект
- выбирать объект по клику мыши в сцене
- показывать `Inspector`
- редактировать `position / rotation / scale`

Как работает выбор объекта:
1. пользователь кликает левой кнопкой мыши
2. из камеры строится луч в 3D-мир
3. луч проверяется на пересечение с AABB объектов
4. выбирается ближайший объект
5. он становится текущим редактируемым объектом

Это пока не суперточный `mesh picking`, а более простой `AABB picking`, но для старта редактора это уже очень полезно.

### 9. ImGui

`Dear ImGui` уже подключён и работает.

Файлы:
- [ImGuiManager.h](/e:/my_proj/OGLE20/src/managers/ImGuiManager.h)
- [ImGuiManager.cpp](/e:/my_proj/OGLE20/src/managers/ImGuiManager.cpp)

Что уже есть:
- debug-окно
- demo window
- editor window
- inspector

### 10. Скрипты

Подключён `Duktape`, то есть в проект уже можно писать `.js`-скрипты.

Файлы:
- [ScriptManager.h](/e:/my_proj/OGLE20/src/managers/ScriptManager.h)
- [ScriptManager.cpp](/e:/my_proj/OGLE20/src/managers/ScriptManager.cpp)

Скрипт может содержать:
- `onStart()`
- `onUpdate(dt)`

Из JS уже доступны функции:
- `log(message)`
- `clearWorld()`
- `spawnCube(name, x, y, z, sx, sy, sz)`
- `setPosition(entityId, x, y, z)`
- `setRotation(entityId, x, y, z)`
- `setScale(entityId, x, y, z)`
- `entityExists(entityId)`

Пример:

```js
var cube = -1;

function onStart() {
    log("Script started");
    cube = spawnCube("BoxFromScript", 0, 1, 0, 1, 1, 1);
}

function onUpdate(dt) {
    if (entityExists(cube)) {
        setRotation(cube, 0, dt * 60.0, 0);
    }
}
```

### 11. Физика

Подключён `Bullet Physics`.

Файлы:
- [PhysicsManager.h](/e:/my_proj/OGLE20/src/managers/PhysicsManager.h)
- [PhysicsManager.cpp](/e:/my_proj/OGLE20/src/managers/PhysicsManager.cpp)

Что умеет сейчас:
- создать физический мир
- задать гравитацию
- добавить box rigid body
- обновлять симуляцию
- переносить результат обратно в `TransformComponent`

Пример:

```cpp
m_physicsManager.AddBoxBody(
    entity,
    {0.5f, 0.5f, 0.5f},
    OGLE::PhysicsBodyType::Dynamic,
    1.0f);
```

### 12. Файловая система

Сейчас уже появился собственный маленький слой файловой системы.

Файлы:
- [FileSystem.h](/e:/my_proj/OGLE20/src/core/FileSystem.h)
- [FileSystem.cpp](/e:/my_proj/OGLE20/src/core/FileSystem.cpp)

Что умеет:
- проверять существование файла
- создавать каталог
- создавать родительский каталог для файла
- читать текстовый файл
- писать текстовый файл
- находить рабочую директорию
- находить директорию `.exe`
- разрешать относительные пути

Это важно, потому что теперь проект уже не живёт как “голый exe”, а умеет нормально работать с файлами проекта.

### 13. Конфигурация

Сейчас в проекте есть конфиг приложения.

Файлы:
- [AppConfig.h](/e:/my_proj/OGLE20/src/config/AppConfig.h)
- [ConfigManager.h](/e:/my_proj/OGLE20/src/config/ConfigManager.h)
- [ConfigManager.cpp](/e:/my_proj/OGLE20/src/config/ConfigManager.cpp)

Что хранится в конфиге:
- заголовок окна
- размер окна
- включён ли редактор
- путь к world-файлу
- нужно ли загружать мир при старте
- нужно ли сохранять дефолтный мир, если файла нет
- нужно ли запускать стартовый скрипт

Пример конфига:

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

## Где что лежит

```text
src/
  config/      # конфиг приложения
  core/        # файловая система и базовые утилиты
  editor/      # редактор
  input/       # ввод
  managers/    # менеджеры подсистем
  models/      # модели и меши
  opengl/      # OpenGL и камера
  ui/          # окно и платформенный код
  world/       # мир, компоненты, world object
  App.*        # основной класс приложения
  main.cpp     # вход в программу
```

## Примерное API проекта

Ниже не “официальная документация генератора”, а карта того, что уже реально есть и чем можно пользоваться.

## `App`

Главный класс приложения.

Основные геттеры:
- `GetCameraManager()`
- `GetEditor()`
- `GetConfigManager()`
- `GetWorldManager()`

## `CameraManager`

Работа с камерой.

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

## `WorldManager`

Работа с активным миром.

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

## `WorldObject`

Удобная оболочка вокруг сущности мира.

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

## `PhysicsManager`

Работа с физикой.

Основные методы:
- `Initialize(worldManager)`
- `SetGravity(...)`
- `AddBoxBody(entity, halfExtents, bodyType, mass)`
- `RemoveBody(entity)`
- `Clear()`
- `Update(deltaTime)`
- `GetBodyCount()`

## `ScriptManager`

Работа со скриптами.

Основные методы:
- `Initialize(worldManager)`
- `ExecuteFile(path)`
- `Update(deltaTime)`

## `Editor`

Редакторская прослойка.

Основные методы:
- `Initialize()`
- `SetEnabled(bool)`
- `Toggle()`
- `IsEnabled()`
- `BuildUi(...)`

Что умеет сейчас:
- выбрать объект
- показать инспектор
- изменить transform выбранного объекта

## `ConfigManager`

Работа с конфигом приложения.

Основные методы:
- `Load()`
- `Save()`
- `LoadOrCreateDefault()`
- `GetConfig()`
- `GetConfigPath()`

## `FileSystem`

Работа с файлами и путями.

Основные методы:
- `Exists(path)`
- `EnsureDirectory(path)`
- `EnsureParentDirectory(path)`
- `ReadTextFile(path, content)`
- `WriteTextFile(path, content)`
- `GetWorkingDirectory()`
- `GetExecutableDirectory()`
- `ResolvePath(path)`

## Как собрать проект

### Быстрая сборка

```powershell
.\build.bat
```

Release:

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

## Как запустить

После сборки:

```powershell
.\bin\OGLE3D.exe
```

## Какие файлы создаёт проект

Обычно используются:
- `config/app_config.json`
- `data/worlds/default_world.json`
- `scripts/*.js`
- `log.txt`

## Что можно делать дальше

Следующие логичные шаги для проекта:
- добавить `Hierarchy`
- расширить `Inspector`
- сделать `MainMenuBar`
- добавить сохранение/загрузку мира из редактора
- сделать подсветку выбранного объекта
- добавить gizmo для перемещения мышкой
- сделать запуск скриптов из редактора
- дать возможность добавлять физику через UI
- сделать нормальную систему ресурсов

## Важная мысль

Этот файл описывает текущее реальное состояние кода.  
Если проект меняется, `README_ru.md` тоже должен обновляться, чтобы документация не отставала от движка.
