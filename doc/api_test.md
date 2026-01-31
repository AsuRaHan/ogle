# Test API

Вспомогательные классы для тестирования и демо.

## TestCube

**Заголовок:** `src/test/TestCube.h`

Тестовый вращающийся куб с материалами и текстурами.

| Метод | Описание |
|-------|----------|
| `TestCube()` | Конструктор. |
| `~TestCube()` | Деструктор. |
| `bool Initialize()` | Создание геометрии куба и тестовых материалов. |
| `void Shutdown()` | Освобождение VAO/VBO/EBO. |
| `void Update(float deltaTime)` | Обновление (вращение). |
| `void Render(float time, Camera* camera)` | Отрисовка куба с текущим материалом и камерой. |
| `void TestTextureSystem()` | Тест системы текстур. |
| `void TestMaterialSystem()` | Тест системы материалов. |
| `void SwitchToNextMaterial()` | Переключить на следующий материал из списка тестовых. |

---

## TestCubeUI

**Заголовок:** `src/test/TestCubeUI.h`

UI для тестового куба (кнопки, счётчики).

| Метод | Описание |
|-------|----------|
| `explicit TestCubeUI(TestCube* cube = nullptr)` | Конструктор. Можно передать указатель на TestCube. |
| `void Initialize()` | Инициализация UI (регистрация окон/кнопок). |
| `void Update(float deltaTime)` | Обновление UI (счётчики и т.д.). |

Приватные поля: указатель на TestCube, время, счётчики нажатий кнопок.
