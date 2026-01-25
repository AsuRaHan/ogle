// src/render/texture/Texture.cpp
#include "Texture.h"
#include <algorithm>

namespace ogle {

// ==================== TextureSettings ====================

GLenum TextureSettings::GetGLMinFilter() const {
    if (!generateMipmaps) {
        return magFilter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR;
    }
    
    switch (minFilter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_LINEAR;
    }
}

GLenum TextureSettings::GetGLMagFilter() const {
    return magFilter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR;
}

GLenum TextureSettings::GetGLWrapS() const {
    switch (wrapS) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_REPEAT;
    }
}

GLenum TextureSettings::GetGLWrapT() const {
    switch (wrapT) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_REPEAT;
    }
}

GLenum TextureSettings::GetGLWrapR() const {
    switch (wrapR) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_REPEAT;
    }
}

// ==================== Texture ====================

Texture::Texture(TextureType type, const std::string& name)
    : m_type(type), m_name(name) {
    glGenTextures(1, &m_id);
}

Texture::~Texture() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        Logger::Debug("Texture destroyed: " + m_name);
    }
}

// ==================== Texture2D ====================

Texture2D::Texture2D(const std::string& name)
    : Texture(TextureType::Texture2D, name) {
}

Texture2D::~Texture2D() {
    // Базовая деструктор уже удаляет текстуру
}

bool Texture2D::Create(int width, int height, TextureFormat format) {
    if (width <= 0 || height <= 0) {
        Logger::Error("Invalid texture dimensions: " + 
                     std::to_string(width) + "x" + std::to_string(height));
        return false;
    }
    
    m_width = width;
    m_height = height;
    m_format = format;
    
    Bind();
    
    // Определяем формат OpenGL
    GLenum internalFormat, dataFormat, dataType;
    switch (format) {
        case TextureFormat::R8:
            internalFormat = GL_R8;
            dataFormat = GL_RED;
            dataType = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGB8:
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
            dataType = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGBA8:
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
            dataType = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGB16F:
            internalFormat = GL_RGB16F;
            dataFormat = GL_RGB;
            dataType = GL_FLOAT;
            break;
        case TextureFormat::RGBA16F:
            internalFormat = GL_RGBA16F;
            dataFormat = GL_RGBA;
            dataType = GL_FLOAT;
            break;
        default:
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
            dataType = GL_UNSIGNED_BYTE;
    }
    
    // Создаем пустую текстуру
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
                width, height, 0, dataFormat, dataType, nullptr);
    
    // Применяем настройки
    SetSettings(m_settings);
    
    Unbind();
    
    Logger::Info("Texture2D created: " + m_name + 
                " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    return true;
}

bool Texture2D::LoadFromFile(const std::string& filepath) {
    // TODO: Реализовать загрузку через stb_image или другую библиотеку
    // Пока создадим простую текстуру
    
    Logger::Warning("LoadFromFile not fully implemented: " + filepath);
    
    // Создаем тестовую текстуру 2x2 пикселя
    const int width = 2;
    const int height = 2;
    
    // Тестовые данные (шахматная доска)
    unsigned char data[] = {
        255, 255, 255, 255,   // белый
        0, 0, 0, 255,         // черный
        0, 0, 0, 255,         // черный
        255, 255, 255, 255    // белый
    };
    
    return LoadFromMemory(data, width, height, 4);
}

bool Texture2D::LoadFromMemory(const unsigned char* data, int width, int height, int channels) {
    if (!data || width <= 0 || height <= 0) {
        return false;
    }
    
    m_width = width;
    m_height = height;
    
    Bind();
    
    // Определяем формат по количеству каналов
    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_RGBA8;
    
    switch (channels) {
        case 1:
            format = GL_RED;
            internalFormat = GL_R8;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG8;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        default:
            Logger::Error("Unsupported number of channels: " + std::to_string(channels));
            Unbind();
            return false;
    }
    
    m_format = (channels == 1) ? TextureFormat::R8 :
               (channels == 3) ? TextureFormat::RGB8 : TextureFormat::RGBA8;
    
    // Загружаем данные
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
                width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // Применяем настройки
    SetSettings(m_settings);
    
    Unbind();
    
    Logger::Info("Texture2D loaded from memory: " + m_name + 
                " (" + std::to_string(width) + "x" + std::to_string(height) + 
                ", channels: " + std::to_string(channels) + ")");
    return true;
}

void Texture2D::Update(const void* data, int x, int y, int width, int height) {
    if (width == 0) width = m_width;
    if (height == 0) height = m_height;
    
    if (x < 0 || y < 0 || x + width > m_width || y + height > m_height) {
        Logger::Error("Texture update out of bounds");
        return;
    }
    
    Bind();
    
    // Определяем формат
    GLenum format = GL_RGBA;
    switch (m_format) {
        case TextureFormat::R8: format = GL_RED; break;
        case TextureFormat::RGB8: format = GL_RGB; break;
        case TextureFormat::RGBA8: format = GL_RGBA; break;
        default: format = GL_RGBA;
    }
    
    // Обновляем суб-область
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, 
                   format, GL_UNSIGNED_BYTE, data);
    
    Unbind();
}

void Texture2D::Bind(GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::SetSettings(const TextureSettings& settings) {
    m_settings = settings;
    
    Bind();
    
    // Устанавливаем параметры фильтрации
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.GetGLMinFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.GetGLMagFilter());
    
    // Устанавливаем параметры обертывания
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, settings.GetGLWrapS());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, settings.GetGLWrapT());
    
    // Если есть цвет границы
    if (settings.wrapS == TextureWrap::ClampToBorder || 
        settings.wrapT == TextureWrap::ClampToBorder) {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, 
                        &settings.borderColor[0]);
    }
    
    // Генерируем мипмапы если нужно
    if (settings.generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    Unbind();
}

// ==================== TextureCube (упрощенная версия) ====================

TextureCube::TextureCube(const std::string& name)
    : Texture(TextureType::TextureCube, name) {
}

TextureCube::~TextureCube() {
}

bool TextureCube::LoadFromFiles(const std::string& front, const std::string& back,
                               const std::string& left, const std::string& right,
                               const std::string& top, const std::string& bottom) {
    Logger::Warning("TextureCube::LoadFromFiles not implemented");
    return false;
}

bool TextureCube::LoadFromSingleFile(const std::string& filepath) {
    Logger::Warning("TextureCube::LoadFromSingleFile not implemented");
    return false;
}

void TextureCube::Bind(GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void TextureCube::Unbind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCube::SetSettings(const TextureSettings& settings) {
    m_settings = settings;
    
    Bind();
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, settings.GetGLMinFilter());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, settings.GetGLMagFilter());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, settings.GetGLWrapS());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, settings.GetGLWrapT());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, settings.GetGLWrapR());
    
    if (settings.generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    
    Unbind();
}

} // namespace ogle