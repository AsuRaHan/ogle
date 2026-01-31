# Systems API

Все системы реализуют интерфейс `ISystem` (GetName, Initialize, Update, Render, Shutdown, OnWindowMessage, OnResize).

## WindowSystem

**Заголовок:** `src/systems/WindowSystem.h`

Создание и обработка окна Win32.

| Метод | Описание |
|-------|----------|
| `WindowSystem(HINSTANCE hInstance, const std::wstring& title, int width = 1280, int height = 720)` | Конструктор. |
| `~WindowSystem()` | Деструктор. |
| `const std::string& GetName() const` | Имя системы: "WindowSystem". |
| `bool Initialize() override` | Инициализация. |
| `bool InitWindow()` | Создание окна и класса. |
| `void Update(float deltaTime) override` | Пусто (окно не тикает). |
| `void Render() override` | Пусто. |
| `void Shutdown() override` | Закрытие окна и освобождение ресурсов. |
| `void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override` | Обработка сообщений окна. |
| `void OnResize(int width, int height) override` | Обработка изменения размера. |
| `HWND GetHWND() const` | Handle окна. |
| `HDC GetHDC() const` | Device context. |
| `int GetWidth() const` | Ширина клиентской области. |
| `int GetHeight() const` | Высота клиентской области. |
| `float GetAspectRatio() const` | Соотношение сторон (width/height). |
| `void AddResizeListener(ResizeCallback cb)` | Подписка на ресайз. |
| `void Show(int nCmdShow = SW_SHOW)` | Показать окно. |

**Тип:** `ResizeCallback = std::function<void(int, int)>`.

---

## RenderSystem

**Заголовок:** `src/systems/RenderSystem.h`

Инициализация OpenGL, очистка экрана, рендер сцены и тестового куба, GUI.

| Метод | Описание |
|-------|----------|
| `explicit RenderSystem(HDC hdc)` | Конструктор (нужен HDC от окна). |
| `~RenderSystem()` | Деструктор. |
| `const std::string& GetName() const` | "RenderSystem". |
| `bool Initialize() override` | Создание OpenGL-контекста, инициализация сцены и тестового куба. |
| `void Update(float deltaTime) override` | Обновление сцены и тестового куба. |
| `void Render() override` | Отрисовка сцены и GUI. |
| `void Shutdown() override` | Освобождение ресурсов. |
| `void OnResize(int width, int height) override` | Изменение viewport и камеры. |
| `void AddRenderer(ISystem* system)` | Добавить систему-рендерер в список. |
| `void RemoveRenderer(ISystem* system)` | Удалить из списка. |
| `void ClearRenderers()` | Очистить список рендереров. |
| `void SetGuiSystem(GuiSystem* guiSystem)` | Установить систему GUI для отрисовки. |
| `void EnableGUI(bool enable)` | Включить/выключить отрисовку GUI. |

---

## InputSystem

**Заголовок:** `src/systems/InputSystem.h`

Обработка ввода (клавиатура, мышь, Raw Input, геймпады) и синхронизация с InputController.

| Метод | Описание |
|-------|----------|
| `InputSystem()` | Конструктор. |
| `~InputSystem()` | Деструктор. |
| `const std::string& GetName() const` | Имя системы. |
| `bool Initialize() override` | Инициализация. |
| `void Update(float deltaTime) override` | Обновление состояний и осей. |
| `void Render() override` | Пусто. |
| `void Shutdown() override` | Освобождение ресурсов. |
| `void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override` | Обработка сообщений окна (клавиши, мышь). |
| `void SetWindowHandle(HWND hwnd)` | Установить handle окна (для Raw Input и т.д.). |

---

## GuiSystem

**Заголовок:** `src/systems/GuiSystem.h`

Инициализация ImGui и отрисовка GUI поверх рендера.

| Метод | Описание |
|-------|----------|
| `GuiSystem()` | Конструктор. |
| `~GuiSystem()` | Деструктор. |
| `const std::string& GetName() const` | Имя системы. |
| `bool Initialize() override` | Инициализация ImGui. |
| `void Update(float deltaTime) override` | Пусто. |
| `void Render() override` | Пусто. |
| `void Shutdown() override` | Завершение ImGui. |
| `void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override` | Передача сообщений в ImGui. |
| `void OnResize(int width, int height) override` | Обработка ресайза. |
| `void RenderUI()` | Отрисовать весь UI (вызывать после начала кадра ImGui). |
| `void SetWindowHandle(HWND hwnd, HDC hdc)` | Привязать окно и контекст. |
| `bool WantCaptureMouse() const` | Нужно ли передавать ввод мыши в ImGui. |
| `bool WantCaptureKeyboard() const` | Нужно ли передавать клавиатуру в ImGui. |
| `bool IsInitialized() const` | Успешно ли инициализирована система. |
