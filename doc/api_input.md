# Input API

## InputController

**Заголовок:** `src/input/InputController.h`

Синглтон контроллера ввода: клавиатура, мышь, геймпад, действия, контексты, оси.

| Метод | Описание |
|-------|----------|
| `static InputController& Get()` | Единственный экземпляр контроллера. |

**Клавиатура**

| Метод | Описание |
|-------|----------|
| `bool IsKeyDown(int vk) const` | Клавиша с кодом `vk` нажата (удерживается). |
| `bool IsKeyPressed(int vk) const` | Клавиша нажата в текущем кадре. |
| `bool IsKeyReleased(int vk) const` | Клавиша отпущена в текущем кадре. |

**Мышь**

| Метод | Описание |
|-------|----------|
| `glm::vec2 GetMousePosition() const` | Текущая позиция мыши. |
| `glm::vec2 GetMouseDelta() const` | Смещение мыши относительно предыдущего кадра. |
| `float GetMouseWheelDelta() const` | Изменение колеса мыши за кадр. |
| `bool IsMouseButtonDown(int button) const` | Кнопка мыши нажата. |
| `bool IsMouseButtonPressed(int button) const` | Кнопка нажата в этом кадре. |
| `bool IsMouseButtonReleased(int button) const` | Кнопка отпущена в этом кадре. |

**Геймпад**

| Метод | Описание |
|-------|----------|
| `bool IsGamepadConnected(int player = 0) const` | Подключён ли геймпад игрока. |
| `bool IsGamepadButtonDown(int player, int button) const` | Кнопка геймпада нажата. |
| `float GetGamepadAxis(int player, int axis) const` | Значение оси геймпада. |

**Input Actions**

| Метод | Описание |
|-------|----------|
| `InputAction* CreateAction(const std::string& name, ActionType type)` | Создать действие по имени и типу. |
| `InputAction* GetAction(const std::string& name)` | Получить действие по имени. |
| `void RemoveAction(const std::string& name)` | Удалить действие. |

**Контексты ввода**

| Метод | Описание |
|-------|----------|
| `void PushContext(const std::string& context)` | Включить контекст (например, "Menu", "Gameplay"). |
| `void PopContext()` | Снять текущий контекст. |
| `const std::string& GetCurrentContext() const` | Текущий активный контекст. |

**Модификаторы**

| Метод | Описание |
|-------|----------|
| `const Modifiers& GetCurrentModifiers() const` | Текущие Ctrl/Shift/Alt/Win. |

**Обновление (для InputSystem)**

| Метод | Описание |
|-------|----------|
| `void UpdateKeyboardState(const bool keyStates[256], ...)` | Обновить состояние клавиатуры. |
| `void UpdateMouseState(const glm::vec2& position, ...)` | Обновить позицию, delta, колесо и кнопки мыши. |
| `void UpdateGamepadState(int player, bool connected, ...)` | Обновить состояние геймпада. |
| `void UpdateModifiers(const Modifiers& modifiers)` | Обновить модификаторы. |
| `void UpdateActions(float deltaTime)` | Обновить все действия (вызывать каждый кадр). |
| `void ResetFrameState()` | Сбросить флаги «нажато/отпущено» в конце кадра. |

**Оси**

| Метод | Описание |
|-------|----------|
| `void RegisterAxis(const std::string& name, const std::vector<AxisBinding>& bindings)` | Зарегистрировать ось по списку биндингов. |
| `void RegisterAxis(const std::string& name, const AxisBinding& binding)` | Зарегистрировать ось с одним биндингом. |
| `float GetAxis(const std::string& name) const` | Текущее значение оси (с сглаживанием). |
| `float GetAxisRaw(const std::string& name) const` | Сырое значение оси. |
| `void UpdateAxes(float deltaTime)` | Обновить оси (вызывать каждый кадр). |
| `void RemoveAxis(const std::string& name)` | Удалить ось. |
| `bool HasAxis(const std::string& name) const` | Есть ли ось с таким именем. |
| `const std::vector<std::string>& GetAxisNames() const` | Список имён осей. |

**Утилиты**

| Метод | Описание |
|-------|----------|
| `size_t GetActionCount() const` | Количество зарегистрированных действий. |
| `bool HasAction(const std::string& name) const` | Есть ли действие с таким именем. |

---

## InputAction

**Заголовок:** `src/input/InputAction.h`

Действие ввода: биндинги клавиш/мыши/геймпада, состояние, колбэки.

| Метод | Описание |
|-------|----------|
| `InputAction(std::string name, ActionType type)` | Конструктор по имени и типу. |

**Биндинги**

| Метод | Описание |
|-------|----------|
| `void AddKey(KeyCode key, Modifiers mods = {}, float scale = 1.0f)` | Добавить клавишу. |
| `void AddMouseButton(MouseButton button, Modifiers mods = {}, float scale = 1.0f)` | Добавить кнопку мыши. |
| `void AddGamepadButton(int player, GamepadButton button, float scale = 1.0f)` | Добавить кнопку геймпада. |
| `void AddGamepadAxis(int player, GamepadAxis axis, float deadzone = 0.1f, float scale = 1.0f)` | Добавить ось геймпада. |
| `void AddAxisPair(KeyCode positive, KeyCode negative, Modifiers mods = {})` | Пара клавиш как ось (например W/S). |

**Обновление и состояние**

| Метод | Описание |
|-------|----------|
| `void Update(float deltaTime)` | Обновить состояние (вызывает InputController). |
| `void ResetFrameState()` | Сбросить флаги кадра. |
| `float Evaluate() const` | Вычислить текущее значение по биндингам (для отладки). |

**Геттеры**

| Метод | Описание |
|-------|----------|
| `const std::string& GetName() const` | Имя действия. |
| `ActionType GetType() const` | Тип действия. |
| `const ActionState& GetState() const` | Текущее состояние (active, value, pressed, released, held и т.д.). |

**Колбэки**

| Метод | Описание |
|-------|----------|
| `void OnPressed(Callback cb)` | Вызов при нажатии. |
| `void OnReleased(Callback cb)` | Вызов при отпускании. |
| `void OnHeld(Callback cb)` | Вызов при удержании. |

`Callback = std::function<void(const ActionState&)>`.

---

## AxisBinding

**Заголовок:** `src/input/AxisBinding.h`

Структура одной привязки к оси ввода (клавиша, мышь, геймпад).

| Метод (статический) | Описание |
|---------------------|----------|
| `static AxisBinding CreateKeyboardKey(KeyCode key, float scale = 1.0f)` | Одна клавиша. |
| `static AxisBinding CreateKeyboardAxis(KeyCode positive, KeyCode negative)` | Пара клавиш (положительная/отрицательная). |
| `static AxisBinding CreateMouseAxis(int axis, float sensitivity = 1.0f)` | Ось мыши (0=X, 1=Y). |
| `static AxisBinding CreateMouseWheel(float sensitivity = 1.0f)` | Колесо мыши. |
| `static AxisBinding CreateGamepadAxis(int player, GamepadAxis axis, float deadzone = 0.1f, float scale = 1.0f)` | Ось геймпада. |

**Поля:** `SourceType type`, union с параметрами (keyboardKey, keyboardPair, mouseAxis, mouseWheel, gamepadAxis), `gravity`, `sensitivity`, `invert`, `snap`.

**InputTypes.h:** перечисления и типы `KeyCode`, `MouseButton`, `GamepadButton`, `GamepadAxis`, `ActionType`, структура `Modifiers`.
