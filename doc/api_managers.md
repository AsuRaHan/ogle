# Managers API

## CameraManager

**Заголовок:** `src/managers/CameraManager.h`

Синглтон менеджера камер: создание, получение по имени, основная камера.

| Метод | Описание |
|-------|----------|
| `static CameraManager& Get()` | Единственный экземпляр менеджера. |
| `Camera* CreateCamera(const std::string& name = "MainCamera")` | Создать камеру с заданным именем. Возвращает указатель на камеру. |
| `Camera* GetCamera(const std::string& name = "MainCamera")` | Получить камеру по имени. Возвращает `nullptr`, если не найдена. |
| `Camera* GetMainCamera()` | Первая созданная (основная) камера. |
| `bool RemoveCamera(const std::string& name)` | Удалить камеру по имени. Возвращает `true` при успехе. |
| `size_t GetCameraCount() const` | Количество камер. |

---

## InputManager

**Заголовок:** `src/managers/InputManager.h`

Синглтон менеджера ввода: обработка клавиш и обновление состояния (в т.ч. для камеры).

| Метод | Описание |
|-------|----------|
| `static InputManager& Get()` | Единственный экземпляр. |
| `void ProcessKey(int keyCode)` | Обработать нажатие клавиши. |
| `void Update(float deltaTime, const std::array<bool, 256>& keyStates, const glm::vec2& mouseDelta, float mouseWheelDelta, bool rightMouseDown)` | Обновить состояние с учётом клавиш, движения мыши, колеса и правой кнопки. |
| `void Initialize()` | Инициализация менеджера. |
