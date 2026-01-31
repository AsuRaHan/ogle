# UI API

## UIController

**Заголовок:** `src/ui/UIController.h`

Синглтон: регистрация окон ImGui, создание кнопок/лейблов, привязка к событиям.

| Метод | Описание |
|-------|----------|
| `static UIController& Get()` | Единственный экземпляр. |

**Окна**

| Метод | Описание |
|-------|----------|
| `void RegisterWindow(const std::string& windowId, const std::string& title, bool* open = nullptr, ImGuiWindowFlags flags = 0)` | Зарегистрировать окно по id и заголовку. |
| `void AddToWindow(const std::string& windowId, const std::string& elementId)` | Добавить элемент в окно по id элемента. |

**Создание элементов**

| Метод | Описание |
|-------|----------|
| `UIButton* CreateButton(const std::string& id, const std::string& label, const ImVec2& size = {0, 0})` | Создать кнопку. |
| `UILabel* CreateLabel(const std::string& id, const std::string& text)` | Создать текст. |
| `UIButton* CreateEventButton(const std::string& id, const std::string& label, const std::string& eventName, const ImVec2& size = {0, 0})` | Кнопка, при нажатии вызывающая событие по имени. |

**Управление элементами**

| Метод | Описание |
|-------|----------|
| `UIElement* GetElement(const std::string& id)` | Получить элемент по id. |
| `template<typename T> T* GetElementAs(const std::string& id)` | Получить элемент с приведением типа (например UIButton*). |
| `void RemoveElement(const std::string& id)` | Удалить элемент. |
| `void ShowElement(const std::string& id)` | Показать элемент. |
| `void HideElement(const std::string& id)` | Скрыть элемент. |

**Рендер и события**

| Метод | Описание |
|-------|----------|
| `void Render()` | Отрисовать все окна и элементы. Вызывать только после ImGui::NewFrame(). |
| `EventSystem& GetEventSystem()` | Доступ к внутренней системе событий (подписка/вызов). |

**Статистика**

| Метод | Описание |
|-------|----------|
| `size_t GetElementCount() const` | Количество элементов. |
| `size_t GetWindowCount() const` | Количество окон. |
| `const std::vector<std::string>* GetWindowElementIds(const std::string& windowId) const` | Список id элементов в окне (или nullptr). |

---

## UIElement

**Заголовок:** `src/ui/UIController.h`

Базовый класс UI-элемента.

| Поле/Метод | Описание |
|------------|----------|
| `std::string id` | Уникальный идентификатор. |
| `std::string label` | Подпись. |
| `bool visible = true` | Видимость. |
| `bool enabled = true` | Включён ли (реакция на ввод). |
| `virtual void Draw() = 0` | Отрисовка элемента. |
| `virtual void Update(float deltaTime)` | Обновление (по умолчанию пусто). |
| `virtual void OnShow()` | Вызов при показе. |
| `virtual void OnHide()` | Вызов при скрытии. |

---

## UIButton

**Заголовок:** `src/ui/UIController.h`

Кнопка на базе UIElement.

| Поле/Метод | Описание |
|------------|----------|
| `std::function<void()> onClick` | Колбэк при нажатии. |
| `ImVec2 size = {0, 0}` | Размер кнопки (0 — авто). |
| `void Draw() override` | Рисует ImGui::Button, при нажатии вызывает onClick. |

---

## UILabel

**Заголовок:** `src/ui/UIController.h`

Текстовая метка.

| Поле/Метод | Описание |
|------------|----------|
| `std::string text` | Текст. |
| `void Draw() override` | Рисует ImGui::Text. |
