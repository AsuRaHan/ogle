# Scene API

## Scene

**Заголовок:** `src/scene/Scene.h`

Сцена на ECS (entt): сущности, иерархия, обновление и рендер.

| Метод | Описание |
|-------|----------|
| `Scene()` | Конструктор. |
| `~Scene()` | Деструктор. |
| `void Initialize(Camera* mainCamera)` | Инициализация сцены, привязка основной камеры. |
| `entt::entity CreateEntity(const std::string& name = "")` | Создать сущность с опциональным именем. |
| `void DestroyEntity(entt::entity e)` | Удалить сущность. |
| `void SetParent(entt::entity child, entt::entity parent)` | Установить родителя в иерархии. |
| `void Update(float deltaTime)` | Обновить сцену (иерархия, физика, компоненты). |
| `void Render(float time, Camera* camera)` | Отрисовать сцену с учётом камеры. |

**Поле:** `entt::registry registry` — реестр сущностей и компонентов.

---

## Mesh

**Заголовок:** `src/scene/Mesh.h`

Компонент меша: геометрия (VAO/VBO/EBO), материал, рендер. Наследует `SceneComponent`.

| Метод | Описание |
|-------|----------|
| `Mesh()` | Конструктор. |
| `~Mesh()` | Деструктор. |
| `bool Initialize()` | Инициализация OpenGL буферов. |
| `void CreateCube()` | Создать тестовый куб. |
| `void Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) override` | Отрисовать меш с заданными матрицами. |
| `float GetBoundingRadius() const` | Радиус ограничивающей сферы. |
| `std::shared_ptr<Material> GetMaterial() const` | Текущий материал. |
| `void SetMaterial(std::shared_ptr<Material> material)` | Установить материал. |
| `void CreateGeometry(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)` | Задать геометрию по вершинам и индексам. |
| `void ComputeBoundingRadius(const std::vector<Vertex>& vertices)` | Вычислить радиус ограничивающей сферы по вершинам. |

**Структура Vertex:** `position`, `normal`, `uv`, `tangent`, `bitangent`, `color`.

---

## Model

**Заголовок:** `src/scene/Model.h`

Загрузка 3D-модели из файла (Assimp) и создание сущностей/мешей в сцене.

| Метод | Описание |
|-------|----------|
| `Model(const std::string& path, Scene* scene)` | Загрузить модель по пути и добавить в сцену. |
| `~Model()` | Деструктор. |
| `entt::entity GetRootEntity() const` | Корневая сущность загруженной иерархии. |

---

## SceneComponent

**Заголовок:** `src/scene/SceneComponent.h`

Базовый компонент сцены: видимость, обновление и рендер.

| Метод | Описание |
|-------|----------|
| `virtual ~SceneComponent()` | Виртуальный деструктор. |
| `virtual void Update(float deltaTime)` | Обновление (по умолчанию пусто). |
| `virtual void Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) = 0` | Отрисовка (обязательная реализация). |

**Поле:** `bool visible = true` — участвует ли в рендере.

---

## Components (Components.h)

**Заголовок:** `src/scene/Components.h`

Структуры компонентов ECS (без классов с публичными методами, кроме встроенных).

| Компонент | Описание |
|-----------|----------|
| **Tag** | `std::string name` — имя сущности. |
| **Transform** | `position`, `rotation`, `scale`; `localMatrix`, `globalMatrix`, `dirty`; методы `MarkDirty()`, `GetLocalMatrix()`. |
| **Hierarchy** | `entt::entity parent`, `std::vector<entt::entity> children` — иерархия. |
| **Bounds** | `center`, `radius`, `globalRadius` — ограничивающая сфера. |
| **PhysicsBody** | `velocity`, `mass` — простая физика. |
