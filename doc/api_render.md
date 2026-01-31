# Render API

## Camera

**Заголовок:** `src/render/Camera.h`

Камера: проекция (перспектива/ортография), позиция, вращение, орбита, матрицы.

| Метод | Описание |
|-------|----------|
| `Camera(const std::string& name = "Camera")` | Конструктор. |
| `~Camera()` | Деструктор. |
| `void Update(float deltaTime)` | Обновить матрицы и векторы направления. |

**Матрицы**

| Метод | Описание |
|-------|----------|
| `const glm::mat4& GetViewMatrix() const` | Матрица вида. |
| `const glm::mat4& GetProjectionMatrix() const` | Матрица проекции. |
| `glm::mat4 GetViewProjection() const` | Произведение projection * view. |

**Проекция**

| Метод | Описание |
|-------|----------|
| `void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip)` | Перспективная проекция. |
| `void SetOrthographic(float size, float aspectRatio, float nearClip, float farClip)` | Ортографическая (по размеру и aspect). |
| `void SetOrthographic(float left, float right, float bottom, float top, float nearClip, float farClip)` | Ортографическая по границам. |

**Трансформация**

| Метод | Описание |
|-------|----------|
| `void SetPosition(const glm::vec3& position)` | Позиция камеры. |
| `void SetRotation(const glm::quat& rotation)` | Поворот кватернионом. |
| `void SetRotation(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f)` | Поворот углами Эйлера. |
| `void Translate(const glm::vec3& translation)` | Сместить камеру. |
| `void Rotate(float yawDelta, float pitchDelta, float rollDelta = 0.0f)` | Повернуть на дельты углов. |
| `void LookAt(const glm::vec3& target, const glm::vec3& up = ...)` | Направить камеру на цель. |

**Движение (FPS)**

| Метод | Описание |
|-------|----------|
| `void MoveForward(float distance)` | Вперёд по направлению взгляда. |
| `void MoveRight(float distance)` | Вправо. |
| `void MoveUp(float distance)` | Вверх. |

**Орбитальная камера**

| Метод | Описание |
|-------|----------|
| `void SetOrbitTarget(const glm::vec3& target)` | Цель орбиты. |
| `void SetOrbitDistance(float distance)` | Расстояние до цели. |
| `void Orbit(float horizontalAngle, float verticalAngle)` | Поворот орбиты по углам. |

**Прочее**

| Метод | Описание |
|-------|----------|
| `void SetAspectRatio(float aspectRatio)` | Соотношение сторон окна. |
| `const glm::vec3& GetPosition() const` | Позиция. |
| `const glm::vec3& GetFront() const` | Вектор «вперёд». |
| `const glm::vec3& GetRight() const` | Вектор «вправо». |
| `const glm::vec3& GetUp() const` | Вектор «вверх». |
| `glm::quat GetRotation() const` | Кватернион поворота. |
| `float GetYaw() / GetPitch() / GetRoll() const` | Углы Эйлера. |
| `Type GetType() / void SetType(Type type)` | Тип: Perspective / Orthographic. |
| `Mode GetMode() / void SetMode(Mode mode)` | Режим: Free, FirstPerson, Orbit, Fixed. |
| `const std::string& GetName() const` | Имя камеры. |
| `void SetMovementSpeed(float speed)` | Скорость движения. |
| `void SetRotationSpeed(float speed)` | Скорость поворота. |
| `void SetMouseSensitivity(float sensitivity)` | Чувствительность мыши. |
| `void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)` | Обработка движения мыши (для FPS). |
| `void ProcessMouseScroll(float yoffset)` | Обработка колеса мыши. |
| `bool IsInFrustum(const glm::vec3& position, float radius) const` | Проверка сферы на попадание во фрустум. |

---

## MaterialController

**Заголовок:** `src/render/MaterialController.h`

Синглтон: создание, хранение, инстансы и встроенные материалы.

| Метод | Описание |
|-------|----------|
| `static MaterialController& Get()` | Единственный экземпляр. |
| `~MaterialController()` | Деструктор. |

**Создание**

| Метод | Описание |
|-------|----------|
| `Material* CreateMaterial(const std::string& name, MaterialType type)` | Создать материал по типу (Basic, PBR, Unlit, Custom). |
| `Material* CreateMaterialFromShader(const std::string& name, const std::string& shaderName)` | Создать материал, привязанный к шейдеру по имени. |

**Управление**

| Метод | Описание |
|-------|----------|
| `Material* GetMaterial(const std::string& name)` | Получить материал по имени. |
| `bool HasMaterial(const std::string& name) const` | Есть ли материал с таким именем. |
| `void RemoveMaterial(const std::string& name)` | Удалить материал. |

**Инстансы**

| Метод | Описание |
|-------|----------|
| `Material* CreateInstance(const std::string& name, const std::string& baseMaterialName)` | Создать инстанс по имени базового материала. |
| `Material* CreateInstance(Material* baseMaterial, const std::string& instanceName = "")` | Создать инстанс по указателю на базовый материал. |

**Встроенные материалы (Builtin)**

| Метод | Описание |
|-------|----------|
| `Material* GetBuiltin(Builtin type)` | Получить встроенный материал (DefaultWhite, DefaultRed, DefaultPBRMetal и т.д.). |
| `void PreloadBuiltinMaterials()` | Предзагрузить все встроенные материалы. |

**Утилиты**

| Метод | Описание |
|-------|----------|
| `void Clear()` | Очистить все материалы. |
| `size_t GetMaterialCount() const` | Количество материалов. |
| `std::vector<std::string> GetMaterialNames() const` | Список имён. |
| `std::vector<Material*> GetMaterialsByShader(const std::string& shaderName) const` | Материалы, использующие шейдер. |
| `void PrintDebugInfo() const` | Вывод отладочной информации. |

**Сохранение/загрузка**

| Метод | Описание |
|-------|----------|
| `bool SaveMaterialToFile(const std::string& name, const std::string& filepath)` | Сохранить материал в файл. |
| `Material* LoadMaterialFromFile(const std::string& filepath, const std::string& materialName = "")` | Загрузить материал из файла. |

---

## Material (базовый класс)

**Заголовок:** `src/render/material/Material.h`

Абстрактный материал: применение к шейдеру, текстуры, параметры, состояние рендера.

| Метод | Описание |
|-------|----------|
| `virtual void Apply(ShaderProgram* shader) = 0` | Применить параметры к шейдеру. |
| `virtual void BindTextures() = 0` | Привязать текстуры к юнитам. |
| `virtual std::unique_ptr<Material> Clone() const = 0` | Клон материала. |
| `virtual MaterialType GetType() const = 0` | Тип материала. |

**Общие виртуальные (цвет, освещение, текстура)**

| Метод | Описание |
|-------|----------|
| `virtual void SetColor(const glm::vec4& color)` | Цвет (Basic и др.). |
| `virtual glm::vec4 GetColor() const` | Текущий цвет. |
| `virtual void SetUseLighting(bool use)` | Включить/выключить освещение. |
| `virtual bool GetUseLighting() const` | Используется ли освещение. |
| `virtual void SetTexture(Texture* texture)` | Текстура по умолчанию (uTexture). |
| `virtual Texture* GetTexture() const` | Текстура по умолчанию. |

**PBR (переопределяются в PBRMaterial)**

| Метод | Описание |
|-------|----------|
| `virtual void SetAlbedo(const glm::vec4& albedo)` | Альбедо. |
| `virtual glm::vec4 GetAlbedo() const` | Альбедо. |
| `virtual void SetMetallic(float metallic)` | Металличность. |
| `virtual float GetMetallic() const` | Металличность. |
| `virtual void SetRoughness(float roughness)` | Шероховатость. |
| `virtual float GetRoughness() const` | Шероховатость. |
| `virtual void SetAO(float ao)` | Ambient occlusion. |
| `virtual float GetAO() const` | AO. |
| `virtual void SetEmission(const glm::vec3& emission)` | Эмиссия. |
| `virtual glm::vec3 GetEmission() const` | Эмиссия. |

**Имя, шейдер, состояние рендера**

| Метод | Описание |
|-------|----------|
| `std::string GetName() const` | Имя материала. |
| `void SetName(const std::string& name)` | Имя. |
| `ShaderProgram* GetShader() const` | Текущий шейдер. |
| `void SetShader(ShaderProgram* shader)` | Установить шейдер. |
| `const RenderState& GetRenderState() const` | Состояние рендера (depth, cull, blend и т.д.). |
| `RenderState& GetRenderState()` | Изменяемое состояние. |
| `void SetRenderState(const RenderState& state)` | Установить состояние. |

**Параметры по имени**

| Метод | Описание |
|-------|----------|
| `virtual void SetFloat/SetVec3/SetVec4/SetInt(const std::string& name, ...)` | Установить параметр. |
| `virtual void SetTexture(const std::string& name, Texture* texture)` | Текстура по имени. |
| `virtual float GetFloat(const std::string& name, float defaultValue = 0.0f) const` | Получить float. |
| `virtual glm::vec3 GetVec3(...) const` | Получить vec3. |
| `virtual glm::vec4 GetVec4(...) const` | Получить vec4. |
| `virtual int GetInt(...) const` | Получить int. |
| `virtual Texture* GetTexture(const std::string& name) const` | Текстура по имени. |
| `virtual void ApplyParameters(ShaderProgram* shader) const` | Применить все параметры к шейдеру. |

**Списки имён параметров (для сериализации)**

| Метод | Описание |
|-------|----------|
| `virtual std::vector<std::string> GetFloatParamNames() const` | Имена float-параметров. |
| `GetIntParamNames / GetVec3ParamNames / GetVec4ParamNames / GetTextureParamNames()` | Аналогично для остальных типов. |

**BasicMaterial** — реализация с цветом, одной текстурой и флагом освещения. Дополнительно: `SetColorWrapped`, `GetColorWrapped`, `SetTextureWrapped`, `GetTextureWrapped`, `SetUseLightingWrapped`, `GetUseLightingWrapped`.

**PBRMaterial** — PBR: альбедо, metallic, roughness, AO, emission; карты Albedo, Normal, MetallicRoughness, AO, Emission. Методы: `SetAlbedoMap`, `SetNormalMap`, `SetMetallicRoughnessMap`, `SetAOMap`, `SetEmissionMap` и соответствующие геттеры.

**MaterialInstance** — декоратор над базовым материалом с переопределением параметров. Методы: `OverrideFloat`, `OverrideVec3`, `OverrideVec4`, `OverrideTexture`; `SetBaseMaterial`, `GetBaseMaterial`.

**RenderState** — структура: depthTest, depthWrite, depthFunc, cullFace, cullMode, blend, blendSrc/blendDst, wireframe, lineWidth; метод `Apply()`.

---

## ShaderController

**Заголовок:** `src/render/ShaderController.h`

Синглтон: создание шейдерных программ из исходников/файлов, текущий шейдер, встроенные, hot-reload.

| Метод | Описание |
|-------|----------|
| `static ShaderController& Get()` | Единственный экземпляр. |

**Создание**

| Метод | Описание |
|-------|----------|
| `std::shared_ptr<ShaderProgram> CreateProgram(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource, const std::string& geometrySource = "")` | Создать программу из строк исходников. |
| `std::shared_ptr<ShaderProgram> CreateProgramFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "")` | Создать программу из файлов. |

**Управление**

| Метод | Описание |
|-------|----------|
| `std::shared_ptr<ShaderProgram> GetProgram(const std::string& name)` | Получить программу по имени. |
| `bool HasProgram(const std::string& name) const` | Есть ли программа с таким именем. |
| `void UseProgram(const std::string& name)` | Активировать программу по имени. |
| `void UseProgram(std::shared_ptr<ShaderProgram> program)` | Активировать программу по указателю. |
| `std::shared_ptr<ShaderProgram> GetCurrentProgram() const` | Текущая активная программа. |
| `void RemoveProgram(const std::string& name)` | Удалить программу. |
| `void Clear()` | Очистить все программы. |

**Встроенные шейдеры**

| Метод | Описание |
|-------|----------|
| `std::shared_ptr<ShaderProgram> GetBuiltin(Builtin type)` | Встроенная программа (BasicColor, BasicTexture, Skybox, Unlit, Wireframe). |
| `void PreloadBuiltinShaders()` | Предзагрузить встроенные. |

**Отладка и hot-reload**

| Метод | Описание |
|-------|----------|
| `Statistics GetStats() const` | Статистика (totalPrograms, compiledShaders, cacheHits/Misses). |
| `void PrintDebugInfo() const` | Вывод отладочной информации. |
| `void WatchShaderFiles(bool enable)` | Включить/выключить отслеживание файлов. |
| `void CheckForUpdates()` | Проверить изменения файлов и перекомпилировать. |

---

## ShaderProgram

**Заголовок:** `src/render/shader/ShaderProgram.h`

Готовая шейдерная программа OpenGL: бинд, uniform-ы, MVP.

| Метод | Описание |
|-------|----------|
| `~ShaderProgram()` | Деструктор. |
| `static std::shared_ptr<ShaderProgram> Create(GLuint id, const std::string& name)` | Создать программу по ID и имени. |
| `void Bind() const` | Активировать программу. |
| `void Unbind() const` | Деактивировать. |
| `void SetInt(const std::string& name, int value)` | Uniform int. |
| `void SetFloat(const std::string& name, float value)` | Uniform float. |
| `void SetVec2/SetVec3/SetVec4(const std::string& name, const glm::vec*& value)` | Uniform vec2/3/4. |
| `void SetMat3/SetMat4(const std::string& name, const glm::mat*& value)` | Uniform mat3/mat4. |
| `void SetBool(const std::string& name, bool value)` | Uniform bool (как int). |
| `void SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)` | Установить model, view, projection за один вызов. |
| `GLuint GetID() const` | ID программы в OpenGL. |
| `const std::string& GetName() const` | Имя. |
| `bool IsValid() const` | ID != 0. |

---

## ShaderLoader

**Заголовок:** `src/render/shader/internal/ShaderLoader.h`

Загрузка и препроцессинг исходников шейдеров.

| Метод | Описание |
|-------|----------|
| `ShaderLoader()` | Конструктор. |
| `std::string LoadFromFile(const std::string& filepath)` | Загрузить исходник из файла. |
| `std::string GetBuiltinSource(const std::string& shaderName)` | Исходник встроенного шейдера. |
| `std::string Preprocess(const std::string& source, const std::string& basePath = "")` | Препроцессинг (в т.ч. #include). |
| `bool HasFileChanged(const std::string& filepath)` | Изменился ли файл с последней загрузки. |

---

## TextureController

**Заголовок:** `src/render/TextureController.h`

Синглтон: создание 2D/кубических текстур, загрузка из файлов, встроенные текстуры.

| Метод | Описание |
|-------|----------|
| `static TextureController& Get()` | Единственный экземпляр. |
| `~TextureController()` | Деструктор. |

**Создание и загрузка**

| Метод | Описание |
|-------|----------|
| `Texture* CreateTexture2D(const std::string& name, int width, int height, const std::string& filepath = "")` | Создать 2D-текстуру (опционально загрузить из файла). |
| `Texture* CreateTextureCube(const std::string& name, const std::vector<std::string>& facePaths)` | Кубическая текстура по путям к граням. |
| `Texture* LoadTexture2D(const std::string& filepath, const std::string& textureName = "")` | Загрузить 2D из файла. |
| `Texture* LoadTextureCube(const std::string& filepath, const std::string& textureName = "")` | Загрузить куб из файла. |

**Управление**

| Метод | Описание |
|-------|----------|
| `Texture* GetTexture(const std::string& name)` | Текстура по имени. |
| `bool HasTexture(const std::string& name) const` | Есть ли текстура с таким именем. |
| `void RemoveTexture(const std::string& name)` | Удалить текстуру. |
| `Texture* GetBuiltin(Builtin type)` | Встроенная текстура (White1x1, Black1x1, Checkerboard и т.д.). |
| `void PreloadBuiltinTextures()` | Предзагрузить встроенные. |
| `void Clear()` | Очистить все текстуры. |
| `size_t GetTextureCount() const` | Количество текстур. |
| `std::vector<std::string> GetTextureNames() const` | Список имён. |
| `std::vector<Texture*> GetTexturesByType(TextureType type) const` | Текстуры по типу. |
| `void PrintDebugInfo() const` | Отладочная информация. |
| `void WatchTextureFiles(bool enable)` | Включить/выключить отслеживание файлов. |
| `void CheckForUpdates()` | Проверить изменения и перезагрузить. |

---

## Texture (базовый класс)

**Заголовок:** `src/render/texture/Texture.h`

Абстрактная текстура: бинд, настройки, загрузка, обновление.

| Метод | Описание |
|-------|----------|
| `virtual void Bind(GLuint unit = 0) = 0` | Привязать к текстурному юниту. |
| `virtual void Unbind() = 0` | Отвязать. |
| `virtual void SetSettings(const TextureSettings& settings) = 0` | Установить фильтрацию и wrap. |
| `std::string GetName() const` | Имя. |
| `void SetName(const std::string& name)` | Имя. |
| `TextureType GetType() const` | Тип (Texture2D, TextureCube и т.д.). |
| `GLuint GetID() const` | ID в OpenGL. |
| `int GetWidth() / GetHeight() / GetDepth() const` | Размеры. |
| `bool IsValid() const` | ID != 0. |
| `virtual bool LoadFromFile(const std::string& filepath) = 0` | Загрузить из файла. |
| `virtual bool LoadFromMemory(const void* data, int width, int height, int channels) = 0` | Загрузить из памяти. |
| `virtual void Update(const void* data, int x, int y, int width, int height) = 0` | Обновить область. |

**Texture2D**

| Метод | Описание |
|-------|----------|
| `Texture2D(const std::string& name = "")` | Конструктор. |
| `bool Create(int width, int height, TextureFormat format = TextureFormat::RGBA8)` | Создать пустую 2D-текстуру. |
| `bool SaveToFile(const std::string& filepath)` | Сохранить в файл. |
| Реализации: `LoadFromFile`, `LoadFromMemory`, `Update`, `Bind`, `Unbind`, `SetSettings`. |

**TextureCube**

| Метод | Описание |
|-------|----------|
| `TextureCube(const std::string& name = "")` | Конструктор. |
| `bool LoadFromFiles(const std::string& front, back, left, right, top, bottom)` | Загрузить по 6 граням. |
| `bool LoadFromSingleFile(const std::string& filepath)` | Загрузить куб из одного файла. |
| `bool Create(int size, TextureFormat format = TextureFormat::RGBA8)` | Создать пустой куб. |
| Реализации виртуальных методов Texture. |

**Типы:** `TextureType`, `TextureFormat`, `TextureFilter`, `TextureWrap`; структура `TextureSettings` с методами `GetGLMinFilter`, `GetGLMagFilter`, `GetGLWrapS/T/R`.

---

## TextureLoader

**Заголовок:** `src/render/texture/TextureLoader.h`

Статические методы загрузки и сохранения изображений (stb_image).

| Метод | Описание |
|-------|----------|
| `static bool LoadImage(const std::string& filepath, std::vector<unsigned char>& outData, int& outWidth, int& outHeight, int& outChannels, bool flipVertical = true, int desiredChannels = 0)` | Загрузить 8-bit изображение. |
| `static bool LoadImageHDR(const std::string& filepath, std::vector<float>& outData, int& outWidth, int& outHeight, int& outChannels, bool flipVertical = true)` | Загрузить HDR (float). |
| `static bool SaveImage(const std::string& filepath, const unsigned char* data, int width, int height, int channels)` | Сохранить 8-bit. |
| `static bool SaveImageHDR(const std::string& filepath, const float* data, int width, int height, int channels)` | Сохранить HDR. |
| `static bool FileExists(const std::string& filepath)` | Существует ли файл. |
| `static std::string GetExtension(const std::string& filepath)` | Расширение файла. |

---

## JsonMaterialSerializer

**Заголовок:** `src/render/material/JsonMaterialSerializer.h`

Сериализация/десериализация материалов в JSON (nlohmann::json).

| Метод | Описание |
|-------|----------|
| `static json Serialize(const Material& material)` | Сериализовать материал. |
| `static json Serialize(const BasicMaterial& material)` | BasicMaterial. |
| `static json Serialize(const PBRMaterial& material)` | PBRMaterial. |
| `static json Serialize(const MaterialInstance& instance)` | MaterialInstance. |
| `static std::unique_ptr<Material> Deserialize(const json& j)` | Десериализовать в Material. |
| `static std::unique_ptr<BasicMaterial> DeserializeBasic(const json& j)` | В BasicMaterial. |
| `static std::unique_ptr<PBRMaterial> DeserializePBR(const json& j)` | В PBRMaterial. |
| `static std::unique_ptr<MaterialInstance> DeserializeInstance(const json& j)` | В MaterialInstance. |
| `static bool SaveToFile(const Material& material, const std::string& filepath)` | Сохранить в файл. |
| `static std::unique_ptr<Material> LoadFromFile(const std::string& filepath)` | Загрузить из файла. |
| `static json SerializeRenderState(const RenderState& state)` | Сериализовать RenderState. |
| `static RenderState DeserializeRenderState(const json& j)` | Десериализовать RenderState. |
| `static json SerializeTextureInfo(const std::string& name, Texture* texture)` | Информация о текстуре для JSON. |
| `static Texture* DeserializeTextureInfo(const json& j)` | Восстановить текстуру из JSON. |
