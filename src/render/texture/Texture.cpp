// src/render/texture/Texture.cpp
#include "Texture.h"
#include <algorithm>
#include <filesystem>

// Включаем stb_image в ТОЛЬКО ОДНОМ .cpp файле
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

// ==================== Вспомогательные функции ====================

void Texture::GetGLFormat(int channels, TextureFormat textureFormat,
                         GLenum& outFormat, GLenum& outInternalFormat, GLenum& outType) {
    switch (textureFormat) {
        case TextureFormat::R8:
            outFormat = GL_RED;
            outInternalFormat = GL_R8;
            outType = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGB8:
            outFormat = GL_RGB;
            outInternalFormat = GL_RGB8;
            outType = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGBA8:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA8;
            outType = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGB16F:
            outFormat = GL_RGB;
            outInternalFormat = GL_RGB16F;
            outType = GL_FLOAT;
            break;
        case TextureFormat::RGBA16F:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA16F;
            outType = GL_FLOAT;
            break;
        default:
            outFormat = GL_RGBA;
            outInternalFormat = GL_RGBA8;
            outType = GL_UNSIGNED_BYTE;
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
    
    GLenum glFormat, internalFormat, dataType;
    GetGLFormat(4, format, glFormat, internalFormat, dataType); // 4 = RGBA
    
    // Создаем пустую текстуру
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
                width, height, 0, glFormat, dataType, nullptr);
    
    // Применяем настройки
    SetSettings(m_settings);
    
    Unbind();
    
    Logger::Info("Texture2D created: " + m_name + 
                " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    return true;
}

bool Texture2D::LoadFromFile(const std::string& filepath) {
    // Используем stb_image для загрузки
    stbi_set_flip_vertically_on_load(true); // OpenGL ожидает начало координат внизу
    
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        Logger::Error("Failed to load texture: " + filepath + 
                     " - " + stbi_failure_reason());
        return false;
    }
    
    bool result = LoadFromMemory(data, width, height, channels);
    
    stbi_image_free(data);
    
    if (result) {
        m_filepath = filepath;
        Logger::Info("Texture loaded from file: " + filepath);
    }
    
    return result;
}

bool Texture2D::LoadFromMemory(const void* data, int width, int height, int channels) {
    if (!data || width <= 0 || height <= 0) {
        return false;
    }
    
    m_width = width;
    m_height = height;
    
    Bind();
    
    // Определяем формат по количеству каналов
    GLenum format, internalFormat;
    switch (channels) {
        case 1:
            format = GL_RED;
            internalFormat = GL_R8;
            m_format = TextureFormat::R8;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG8;
            m_format = TextureFormat::RGBA8; // Нет RG8 в enum, используем RGBA8
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            m_format = TextureFormat::RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            m_format = TextureFormat::RGBA8;
            break;
        default:
            Logger::Error("Unsupported number of channels: " + std::to_string(channels));
            Unbind();
            return false;
    }
    
    // Загружаем данные
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
                width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // Применяем настройки (генерируем мипмапы если нужно)
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
    GLenum format;
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
    if (unit >= 32) { // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS обычно 32-80
        Logger::Error("Texture unit out of range: " + std::to_string(unit));
        return;
    }
    
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
    
    // Генерируем мипмапы если нужно И если есть данные
    if (settings.generateMipmaps && m_width > 0 && m_height > 0) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    Unbind();
}

bool Texture2D::SaveToFile(const std::string& filepath) {
    // Для сохранения нужно прочитать данные из GPU
    Bind();
    
    std::vector<unsigned char> data(m_width * m_height * 4); // RGBA
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    
    Unbind();
    
    // Определяем расширение
    std::string ext = std::filesystem::path(filepath).extension().string();
    
    if (ext == ".png") {
        return stbi_write_png(filepath.c_str(), m_width, m_height, 4, data.data(), m_width * 4) != 0;
    } else if (ext == ".jpg" || ext == ".jpeg") {
        return stbi_write_jpg(filepath.c_str(), m_width, m_height, 4, data.data(), 90) != 0;
    } else if (ext == ".bmp") {
        return stbi_write_bmp(filepath.c_str(), m_width, m_height, 4, data.data()) != 0;
    }
    
    Logger::Error("Unsupported save format: " + ext);
    return false;
}

// ==================== TextureCube ====================

TextureCube::TextureCube(const std::string& name)
    : Texture(TextureType::TextureCube, name) {
}

TextureCube::~TextureCube() {
}

// Реализации TextureCube (упрощенные, но рабочие)
bool TextureCube::LoadFromFile(const std::string& filepath) {
    Logger::Warning("TextureCube::LoadFromFile not implemented - use LoadFromSingleFile for HDR cubemaps");
    return false;
}

bool TextureCube::LoadFromMemory(const void* data, int width, int height, int channels) {
    Logger::Warning("TextureCube::LoadFromMemory not implemented");
    return false;
}

void TextureCube::Update(const void* data, int x, int y, int width, int height) {
    Logger::Warning("TextureCube::Update not implemented");
}

void TextureCube::Bind(GLuint unit) {
    if (unit >= 32) {
        Logger::Error("Texture unit out of range: " + std::to_string(unit));
        return;
    }
    
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
    
    if (settings.generateMipmaps && m_width > 0) {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    
    Unbind();
}

bool TextureCube::LoadFromFiles(const std::string& front, const std::string& back,
                               const std::string& left, const std::string& right,
                               const std::string& top, const std::string& bottom) {
    std::vector<std::string> faces = {right, left, top, bottom, front, back};
    GLenum faceTargets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    
    Bind();
    
    for (int i = 0; i < 6; i++) {
        int width, height, channels;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            Logger::Error("Failed to load cube face: " + faces[i]);
            Unbind();
            return false;
        }
        
        if (i == 0) {
            m_width = width;
            m_height = height;
        } else if (width != m_width || height != m_height) {
            Logger::Error("Cube face dimensions mismatch");
            stbi_image_free(data);
            Unbind();
            return false;
        }
        
        GLenum format;
        switch (channels) {
            case 1: format = GL_RED; break;
            case 2: format = GL_RG; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default: format = GL_RGB;
        }
        
        glTexImage2D(faceTargets[i], 0, format, width, height, 0, 
                    format, GL_UNSIGNED_BYTE, data);
        
        stbi_image_free(data);
    }
    
    SetSettings(m_settings);
    Unbind();
    
    Logger::Info("Cube texture loaded from 6 files");
    return true;
}

bool TextureCube::LoadFromSingleFile(const std::string& filepath) {
    // Для HDR кубических карт
    stbi_set_flip_vertically_on_load(false);
    
    int width, height, channels;
    float* data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        Logger::Error("Failed to load HDR cubemap: " + filepath);
        return false;
    }
    
    // Простая реализация - создаем кубическую текстуру из HDR
    // В реальности нужна конвертация в 6 граней
    
    stbi_image_free(data);
    Logger::Warning("TextureCube::LoadFromSingleFile - HDR to cubemap conversion not implemented");
    return false;
}

bool TextureCube::Create(int size, TextureFormat format) {
    if (size <= 0) return false;
    
    m_width = m_height = size;
    m_format = format;
    
    Bind();
    
    GLenum glFormat, internalFormat, dataType;
    GetGLFormat(4, format, glFormat, internalFormat, dataType);
    
    // Создаем 6 пустых граней
    GLenum faces[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    
    for (GLenum face : faces) {
        glTexImage2D(face, 0, internalFormat, size, size, 0, 
                    glFormat, dataType, nullptr);
    }
    
    SetSettings(m_settings);
    Unbind();
    
    Logger::Info("Cube texture created: " + m_name + " (" + std::to_string(size) + "x" + std::to_string(size) + ")");
    return true;
}

} // namespace ogle