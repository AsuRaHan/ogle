# Core и Log API

## Engine

**Заголовок:** `src/core/Engine.h`

Главный класс приложения: инициализация, регистрация систем, игровой цикл.

| Метод | Описание |
|-------|----------|
| `explicit Engine(HINSTANCE hInstance)` | Конструктор. Принимает handle экземпляра приложения (Windows). |
| `~Engine()` | Деструктор. |
| `bool Initialize()` | Инициализация движка и всех зарегистрированных систем. Возвращает `true` при успехе. |
| `int Run()` | Запуск главного цикла. Возвращает код выхода. |
| `void Shutdown()` | Корректное завершение работы движка. |
| `T* RegisterSystem<T>(Args&&... args)` | Регистрирует систему типа `T` с переданными аргументами. Возвращает указатель на созданную систему. |
| `T* GetSystem<T>() const` | Возвращает указатель на зарегистрированную систему типа `T` или `nullptr`. |
| `float GetDeltaTime() const` | Время в секундах с предыдущего кадра. |

---

## ISystem

**Заголовок:** `src/core/ISystem.h`

Интерфейс системы движка. Все системы (Window, Render, Input, Gui) реализуют этот интерфейс.

| Метод | Описание |
|-------|----------|
| `virtual ~ISystem()` | Виртуальный деструктор. |
| `virtual const std::string& GetName() const = 0` | Имя системы (для логов и отладки). |
| `virtual bool Initialize() = 0` | Инициализация системы. Возвращает `true` при успехе. |
| `virtual void Update(float deltaTime) = 0` | Обновление логики каждый кадр. |
| `virtual void Render() = 0` | Отрисовка (если применимо). |
| `virtual void Shutdown() = 0` | Освобождение ресурсов при выходе. |
| `virtual void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam)` | Обработка сообщения окна (по умолчанию пусто). |
| `virtual void OnResize(int width, int height)` | Вызов при изменении размера окна (по умолчанию пусто). |

---

## EventSystem

**Заголовок:** `src/core/EventSystem.h`

Система событий по имени: подписка, отписка, вызов.

| Метод | Описание |
|-------|----------|
| `EventID Subscribe(const std::string& eventName, Callback callback, int priority = 0)` | Подписка на событие. Возвращает ID подписки. `priority` — чем выше, тем раньше вызов. |
| `void Unsubscribe(const std::string& eventName, EventID id)` | Отписка по имени события и ID. |
| `void Emit(const std::string& eventName)` | Вызов всех подписчиков события. |
| `bool HasListeners(const std::string& eventName) const` | Есть ли подписчики на событие. |
| `void ClearEvent(const std::string& eventName)` | Удалить всех подписчиков события. |
| `void ClearAll()` | Очистить все события и подписчиков. |

**Типы:** `EventID = size_t`, `Callback = std::function<void()>`.

---

## Logger

**Заголовок:** `src/log/Logger.h`

Статический логгер с уровнями.

| Метод | Описание |
|-------|----------|
| `static void Debug(const std::string& message)` | Сообщение уровня Debug. |
| `static void Info(const std::string& message)` | Информационное сообщение. |
| `static void Warning(const std::string& message)` | Предупреждение. |
| `static void Error(const std::string& message)` | Ошибка. |
| `static void Success(const std::string& message)` | Успех. |
| `static void Log(LogLevel level, const std::string& message)` | Запись с указанным уровнем. |

**Enum:** `LogLevel::Debug`, `Info`, `Warning`, `Error`, `Success`.
