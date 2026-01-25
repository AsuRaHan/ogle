// src/render/texture/Texture.h
#pragma once
#include <glad/gl.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "log/Logger.h"

namespace ogle {

// Типы текстур
enum class TextureType {
    Texture2D,
    TextureCube,
    Texture3D,
    Texture2DArray
};

// Форматы текстур
enum class TextureFormat {
    R8,
    RGB8,
    RGBA8,
    RGB16F,
    RGBA16F,
    RGB32F,
    RGBA32F,
    Depth16,
    Depth24,
    Depth32F
};

// Параметры фильтрации
enum class TextureFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

// Параметры обертывания
enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

// Настройки текстуры
struct TextureSettings {
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureWrap wrapS = TextureWrap::Repeat;
    TextureWrap wrapT = TextureWrap::Repeat;
    TextureWrap wrapR = TextureWrap::Repeat;
    bool generateMipmaps = true;
    bool srgb = false;
    glm::vec4 borderColor = {0,0,0,1};
    
    // Конвертация в OpenGL константы
    GLenum GetGLMinFilter() const;
    GLenum GetGLMagFilter() const;
    GLenum GetGLWrapS() const;
    GLenum GetGLWrapT() const;
    GLenum GetGLWrapR() const;
};

// Базовый класс текстуры
class Texture {
public:
    virtual ~Texture();
    
    // Виртуальные методы
    virtual void Bind(GLuint unit = 0) = 0;
    virtual void Unbind() = 0;
    virtual void SetSettings(const TextureSettings& settings) = 0;
    
    // Общие методы
    std::string GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    
    TextureType GetType() const { return m_type; }
    GLuint GetID() const { return m_id; }
    
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetDepth() const { return m_depth; }
    
    bool IsValid() const { return m_id != 0; }
    
protected:
    Texture(TextureType type, const std::string& name = "");
    
    GLuint m_id = 0;
    TextureType m_type;
    std::string m_name;
    
    int m_width = 0;
    int m_height = 0;
    int m_depth = 0;
    TextureFormat m_format = TextureFormat::RGBA8;
};

// 2D текстура
class Texture2D : public Texture {
public:
    Texture2D(const std::string& name = "");
    ~Texture2D() override;
    
    // Создание
    bool Create(int width, int height, TextureFormat format = TextureFormat::RGBA8);
    bool LoadFromFile(const std::string& filepath);
    bool LoadFromMemory(const unsigned char* data, int width, int height, int channels);
    
    // Обновление данных
    void Update(const void* data, int x = 0, int y = 0, int width = 0, int height = 0);
    
    // Методы Texture
    void Bind(GLuint unit = 0) override;
    void Unbind() override;
    void SetSettings(const TextureSettings& settings) override;
    
    // Утилиты
    bool SaveToFile(const std::string& filepath);
    
private:
    TextureSettings m_settings;
};

// Кубическая текстура (для skybox)
class TextureCube : public Texture {
public:
    TextureCube(const std::string& name = "");
    ~TextureCube() override;
    
    // Загрузка 6 граней
    bool LoadFromFiles(const std::string& front, const std::string& back,
                      const std::string& left, const std::string& right,
                      const std::string& top, const std::string& bottom);
    
    bool LoadFromSingleFile(const std::string& filepath); // Загрузка HDR/кубической карты
    
    // Методы Texture
    void Bind(GLuint unit = 0) override;
    void Unbind() override;
    void SetSettings(const TextureSettings& settings) override;
    
private:
    TextureSettings m_settings;
};

} // namespace ogle