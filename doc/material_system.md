# 📚 Документация по системе материалов Ogle Engine

## 📋 Оглавление
1. [Базовые понятия](#базовые-понятия)
2. [Создание материалов](#создание-материалов)
3. [Типы материалов](#типы-материалов)
4. [Работа с цветами](#работа-с-цветами)
5. [Работа с текстурами](#работа-с-текстурами)
6. [Material Instances](#material-instances)
7. [Состояние рендеринга](#состояние-рендеринга)
8. [Примеры использования](#примеры-использования)

---

## 🎯 Базовые понятия

### Основные компоненты:
- **Material** - абстрактный базовый класс
- **BasicMaterial** - материал с цветом/текстурой + освещение
- **PBRMaterial** - PBR материал (металл/шероховатость)
- **MaterialInstance** - инстанс материала для переопределения параметров

### Контроллеры:
```cpp
// Синглтоны для управления
auto& materialCtrl = MaterialController::Get();
auto& shaderCtrl = ShaderController::Get();
auto& textureCtrl = TextureController::Get();
```

---

## 🏗️ Создание материалов

### 1. Простой цветной материал:
```cpp
#include "render/MaterialController.h"
#include "render/ShaderController.h"
#include "render/material/Material.h"

// Получаем контроллер
auto& materialCtrl = MaterialController::Get();

// Создаем красный материал
auto* redMaterial = materialCtrl.CreateMaterial("RedMaterial", MaterialType::Basic);

// Устанавливаем шейдер (обязательно!)
auto shader = ShaderController::Get().GetBuiltin(ShaderController::Builtin::BasicColor);
redMaterial->SetShader(shader.get());

// Устанавливаем цвет
redMaterial->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Красный

// Включаем освещение
redMaterial->SetUseLighting(true);

// Направление света
redMaterial->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
```

### 2. Получение существующего материала:
```cpp
// Получить по имени
auto* material = MaterialController::Get().GetMaterial("RedMaterial");
if (material) {
    // Работаем с материалом
}
```

### 3. Удаление материала:
```cpp
MaterialController::Get().RemoveMaterial("RedMaterial");
```

---

## 📊 Типы материалов

### MaterialType:
```cpp
enum class MaterialType {
    Basic,      // Простой материал (цвет/текстура)
    PBR,        // PBR материал
    Unlit,      // Без освещения
    Custom      // Пользовательский
};
```

---

## 🎨 Работа с цветами

### Для BasicMaterial:
```cpp
auto* material = MaterialController::Get().CreateMaterial("MyMaterial", MaterialType::Basic);

// Установка цвета (4 варианта):
material->SetColor(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Оранжевый
material->SetVec4("uColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // Зеленый
material->SetVec3("uColor", glm::vec3(0.0f, 0.0f, 1.0f)); // Синий (альфа=1 по умолчанию)

// Получение цвета:
glm::vec4 color = material->GetColor();
glm::vec3 colorRGB = material->GetVec3("uColor");

// Включение/выключение освещения:
material->SetUseLighting(true);  // С освещением
material->SetUseLighting(false); // Без освещения (плоское затенение)
```

### Настройка освещения:
```cpp
// Источник света (направление)
material->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));

// Или из камеры:
material->SetVec3("uLightDir", camera->GetFront());
```

---

## 🖼️ Работа с текстурами

### 1. Создание текстурированного материала:
```cpp
#include "render/TextureController.h"

auto& textureCtrl = TextureController::Get();
auto& materialCtrl = MaterialController::Get();

// Загружаем текстуру
Texture* brickTexture = textureCtrl.LoadTexture2D("textures/brick.png");

// Создаем материал
auto* texturedMat = materialCtrl.CreateMaterial("BrickWall", MaterialType::Basic);

// Выбираем шейдер для текстур
auto texShader = ShaderController::Get().GetBuiltin(ShaderController::Builtin::BasicTexture);
texturedMat->SetShader(texShader.get());

// Устанавливаем текстуру
texturedMat->SetTexture(brickTexture);

// Указываем использовать текстуру (а не цвет)
texturedMat->SetInt("uUseColor", 0); // 0 = текстура, 1 = цвет

// Освещение
texturedMat->SetUseLighting(true);
texturedMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
```

### 2. Встроенные текстуры:
```cpp
// Получить встроенную текстуру
Texture* whiteTex = textureCtrl.GetBuiltin(TextureController::Builtin::White1x1);
Texture* checkerTex = textureCtrl.GetBuiltin(TextureController::Builtin::Checkerboard);
Texture* normalTex = textureCtrl.GetBuiltin(TextureController::Builtin::NormalFlat);

// Полный список встроенных текстур:
// - White1x1, Black1x1, Gray1x1
// - Red1x1, Green1x1, Blue1x1
// - Checkerboard (шахматная доска)
// - NormalFlat (плоская нормаль)
// - DefaultSkybox
```

### 3. Несколько текстур в материале:
```cpp
// Основная текстура
texturedMat->SetTexture(diffuseTex);

// Дополнительные текстуры (по именам)
texturedMat->SetTexture("uNormalMap", normalTex);
texturedMat->SetTexture("uSpecularMap", specularTex);
```

### 4. Кубические текстуры (skybox):
```cpp
// Загрузка skybox из 6 файлов
Texture* skybox = textureCtrl.CreateTextureCube("Skybox", {
    "skybox/right.jpg",
    "skybox/left.jpg", 
    "skybox/top.jpg",
    "skybox/bottom.jpg",
    "skybox/front.jpg",
    "skybox/back.jpg"
});

// Материал для skybox
auto* skyboxMat = materialCtrl.CreateMaterial("SkyboxMaterial", MaterialType::Basic);
skyboxMat->SetShader(ShaderController::Get().GetBuiltin(ShaderController::Builtin::Skybox).get());
skyboxMat->SetTexture(skybox);
```

---

## 🎭 Material Instances

### Создание инстанса:
```cpp
// Базовый материал
auto* baseMat = materialCtrl.CreateMaterial("BaseMetal", MaterialType::Basic);
baseMat->SetColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

// Создаем инстанс с переопределением цвета
auto* instance = materialCtrl.CreateInstance("RustedMetal", "BaseMetal");
instance->SetColor(glm::vec4(0.5f, 0.4f, 0.3f, 1.0f)); // Ржавый цвет

// Или создаем инстанс напрямую
auto* instance2 = materialCtrl.CreateInstance(baseMat, "ScratchedMetal");
```

### Переопределение параметров:
```cpp
MaterialInstance* instance = dynamic_cast<MaterialInstance*>(material);

if (instance) {
    // Переопределение значений
    instance->OverrideFloat("uRoughness", 0.8f);
    instance->OverrideVec3("uColor", glm::vec3(1.0f, 0.5f, 0.0f));
    instance->OverrideTexture("uAlbedoMap", newTexture);
    
    // Получение базового материала
    Material* base = instance->GetBaseMaterial();
}
```

---

## ⚙️ Состояние рендеринга

### Настройка RenderState:
```cpp
auto* material = materialCtrl.CreateMaterial("Transparent", MaterialType::Basic);

// Получаем состояние рендеринга
RenderState& state = material->GetRenderState();

// Depth test
state.depthTest = true;
state.depthWrite = true;
state.depthFunc = RenderState::Less;

// Face culling
state.cullFace = true;
state.cullMode = RenderState::Back; // или Front, None

// Blending (прозрачность)
state.blend = true;
state.blendSrc = RenderState::SrcAlpha;
state.blendDst = RenderState::OneMinusSrcAlpha;

// Wireframe режим
state.wireframe = true;
state.lineWidth = 2.0f;
```

### Быстрые пресеты:
```cpp
// Непрозрачный материал
void SetupOpaqueMaterial(Material* mat) {
    auto& state = mat->GetRenderState();
    state.depthTest = true;
    state.depthWrite = true;
    state.cullFace = true;
    state.blend = false;
    state.wireframe = false;
}

// Прозрачный материал
void SetupTransparentMaterial(Material* mat) {
    auto& state = mat->GetRenderState();
    state.depthTest = true;
    state.depthWrite = false; // Не записывать в depth buffer
    state.cullFace = false;   // Обычно отключаем для прозрачных
    state.blend = true;
    state.blendSrc = RenderState::SrcAlpha;
    state.blendDst = RenderState::OneMinusSrcAlpha;
}

// Wireframe материал
void SetupWireframeMaterial(Material* mat) {
    auto& state = mat->GetRenderState();
    state.wireframe = true;
    state.lineWidth = 1.5f;
    state.cullFace = false; // Чтобы видеть задние грани
}
```

---

## 📝 Примеры использования

### Пример 1: Полный цикл работы с материалом
```cpp
// Инициализация
void InitMaterials() {
    auto& matCtrl = MaterialController::Get();
    auto& shaderCtrl = ShaderController::Get();
    auto& texCtrl = TextureController::Get();
    
    // 1. Цветной материал
    auto* colorMat = matCtrl.CreateMaterial("RedPlastic", MaterialType::Basic);
    colorMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
    colorMat->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    colorMat->SetUseLighting(true);
    colorMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
    
    // 2. Текстурированный материал
    Texture* rockTex = texCtrl.LoadTexture2D("textures/rock.png");
    auto* texMat = matCtrl.CreateMaterial("RockWall", MaterialType::Basic);
    texMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicTexture).get());
    texMat->SetTexture(rockTex);
    texMat->SetInt("uUseColor", 0);
    texMat->SetUseLighting(true);
    texMat->SetVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.5f));
    
    // 3. Wireframe материал
    auto* wireMat = matCtrl.CreateMaterial("Wireframe", MaterialType::Basic);
    wireMat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
    wireMat->SetColor(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
    wireMat->SetUseLighting(false);
    wireMat->GetRenderState().wireframe = true;
    wireMat->GetRenderState().lineWidth = 1.5f;
}

// Рендеринг
void RenderObject(Material* material, const glm::mat4& model, Camera* camera) {
    if (!material || !material->GetShader()) return;
    
    auto* shader = material->GetShader();
    shader->Bind();
    
    // Устанавливаем матрицы
    shader->SetMat4("uModel", model);
    shader->SetMat4("uView", camera->GetViewMatrix());
    shader->SetMat4("uProjection", camera->GetProjectionMatrix());
    
    // Применяем материал
    material->Apply(shader);
    
    // Применяем состояние рендеринга
    material->GetRenderState().Apply();
    
    // Рисуем геометрию
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    shader->Unbind();
}
```

### Пример 2: PBR материал
```cpp
// Создание PBR материала
auto* pbrMat = materialCtrl.CreateMaterial("Gold", MaterialType::PBR);
if (pbrMat->GetType() == MaterialType::PBR) {
    PBRMaterial* pbr = dynamic_cast<PBRMaterial*>(pbrMat);
    
    // Базовые свойства
    pbr->SetAlbedo(glm::vec4(1.0f, 0.8f, 0.0f, 1.0f)); // Золотой цвет
    pbr->SetMetallic(1.0f);    // Полностью металлический
    pbr->SetRoughness(0.3f);   // Гладкий
    pbr->SetAO(1.0f);          // Полная окклюзия
    pbr->SetEmission(glm::vec3(0.0f, 0.0f, 0.0f)); // Нет свечения
    
    // Текстуры PBR
    pbr->SetAlbedoMap(albedoTex);
    pbr->SetNormalMap(normalTex);
    pbr->SetMetallicRoughnessMap(metalRoughTex);
    pbr->SetAOMap(aoTex);
    pbr->SetEmissionMap(emissionTex);
}
```

### Пример 3: Динамическое переключение материалов
```cpp
class MaterialSwitcher {
private:
    std::vector<Material*> m_materials;
    size_t m_currentIndex = 0;
    
public:
    void AddMaterial(Material* mat) {
        m_materials.push_back(mat);
    }
    
    Material* GetCurrent() {
        return m_materials.empty() ? nullptr : m_materials[m_currentIndex];
    }
    
    void Next() {
        if (!m_materials.empty()) {
            m_currentIndex = (m_currentIndex + 1) % m_materials.size();
            Logger::Info("Switched to material: " + GetCurrent()->GetName());
        }
    }
    
    void Prev() {
        if (!m_materials.empty()) {
            m_currentIndex = (m_currentIndex - 1 + m_materials.size()) % m_materials.size();
            Logger::Info("Switched to material: " + GetCurrent()->GetName());
        }
    }
};
```

### Пример 4: Hot Reload материалов
```cpp
// В основном цикле
void Update(float deltaTime) {
    // Проверяем обновления шейдеров
    ShaderController::Get().CheckForUpdates();
    
    // Проверяем обновления текстур
    TextureController::Get().CheckForUpdates();
    
    // Можно добавить hot-reload для JSON материалов
    static float reloadTimer = 0.0f;
    reloadTimer += deltaTime;
    
    if (reloadTimer > 2.0f) { // Каждые 2 секунды
        ReloadChangedMaterials();
        reloadTimer = 0.0f;
    }
}
```

---

## 💡 Советы и лучшие практики

### 1. **Именование материалов:**
```cpp
// Хорошо:
CreateMaterial("Building_BrickWall", MaterialType::Basic);
CreateMaterial("Character_Main", MaterialType::Basic);
CreateMaterial("Weapon_Metal_PBR", MaterialType::PBR);

// Плохо:
CreateMaterial("mat1", MaterialType::Basic);
CreateMaterial("texture", MaterialType::Basic);
```

### 2. **Группировка материалов:**
```cpp
// Материалы для зданий
std::vector<Material*> buildingMaterials = {
    materialCtrl.GetMaterial("Building_BrickWall"),
    materialCtrl.GetMaterial("Building_Wood"),
    materialCtrl.GetMaterial("Building_Glass")
};

// Материалы для персонажей
std::vector<Material*> characterMaterials = {
    materialCtrl.GetMaterial("Character_Skin"),
    materialCtrl.GetMaterial("Character_Cloth"),
    materialCtrl.GetMaterial("Character_Metal")
};
```

### 3. **Кэширование материалов:**
```cpp
class MaterialCache {
private:
    std::unordered_map<std::string, Material*> m_cache;
    
public:
    Material* GetOrCreate(const std::string& name, 
                         const std::function<Material*()>& creator) {
        auto it = m_cache.find(name);
        if (it != m_cache.end()) {
            return it->second;
        }
        
        Material* mat = creator();
        if (mat) {
            m_cache[name] = mat;
        }
        return mat;
    }
};
```

### 4. **Отладка материалов:**
```cpp
void DebugMaterial(Material* material) {
    if (!material) return;
    
    Logger::Info("=== Material Debug ===");
    Logger::Info("Name: " + material->GetName());
    Logger::Info("Type: " + std::to_string(static_cast<int>(material->GetType())));
    Logger::Info("Shader: " + (material->GetShader() ? material->GetShader()->GetName() : "None"));
    
    // Проверка параметров
    glm::vec4 color = material->GetColor();
    Logger::Info("Color: " + 
        std::to_string(color.r) + ", " +
        std::to_string(color.g) + ", " +
        std::to_string(color.b) + ", " +
        std::to_string(color.a));
    
    Logger::Info("UseLighting: " + std::string(material->GetUseLighting() ? "Yes" : "No"));
    
    if (material->GetTexture()) {
        Logger::Info("Texture: " + material->GetTexture()->GetName());
    }
}
```

---

## 🚀 Быстрый старт

Минимальный пример работы с материалом:

```cpp
// 1. Инициализация
auto& matCtrl = MaterialController::Get();
auto& shaderCtrl = ShaderController::Get();

// 2. Создание материала
auto* mat = matCtrl.CreateMaterial("SimpleRed", MaterialType::Basic);
mat->SetShader(shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor).get());
mat->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
mat->SetUseLighting(true);
mat->SetVec3("uLightDir", glm::vec3(0.0f, 1.0f, 0.0f));

// 3. Использование в рендеринге
void Render() {
    auto* shader = mat->GetShader();
    shader->Bind();
    
    shader->SetMat4("uModel", modelMatrix);
    shader->SetMat4("uView", viewMatrix);
    shader->SetMat4("uProjection", projectionMatrix);
    
    mat->Apply(shader);
    mat->GetRenderState().Apply();
    
    // Рисуем объект...
}
```

---

Система материалов Ogle Engine предоставляет гибкий и мощный инструментарий для работы с материалами различной сложности - от простых цветных поверхностей до сложных PBR материалов с несколькими текстурами.