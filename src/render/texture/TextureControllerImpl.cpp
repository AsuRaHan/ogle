// src/render/texture/TextureControllerImpl.cpp
#include "TextureControllerImpl.h"
#include "Texture.h"
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

namespace ogle {

TextureControllerImpl::TextureControllerImpl() {
    Logger::Info("TextureControllerImpl initialized");
    PreloadBuiltinTextures();
}

TextureControllerImpl::~TextureControllerImpl() {
    Clear();
    Logger::Info("TextureControllerImpl shutdown");
}

Texture* TextureControllerImpl::CreateTexture2D(const std::string& name, 
                                               int width, int height,
                                               const std::string& filepath) {
    // Проверяем уникальность
    if (HasTexture(name)) {
        Logger::Warning("Texture already exists: " + name);
        return GetTexture(name);
    }
    
    // Создаем текстуру
    auto texture = std::make_unique<Texture2D>(name);
    
    if (!filepath.empty()) {
        // Загружаем из файла
        if (!texture->LoadFromFile(filepath)) {
            Logger::Error("Failed to load texture from file: " + filepath);
            return nullptr;
        }
        m_stats.loadedFromFile++;
    } else {
        // Создаем пустую текстуру
        if (!texture->Create(width, height)) {
            Logger::Error("Failed to create texture: " + name);
            return nullptr;
        }
        m_stats.createdProcedural++;
    }
    
    // Сохраняем
    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++;
    m_stats.texture2DCount++;
    
    Logger::Info("Texture2D created: " + name + 
                " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    
    return ptr;
}

Texture* TextureControllerImpl::CreateTextureCube(const std::string& name,
                                                 const std::vector<std::string>& facePaths) {
    Logger::Warning("CreateTextureCube not implemented yet");
    return nullptr;
}

Texture* TextureControllerImpl::LoadTexture2D(const std::string& filepath, 
                                             const std::string& textureName) {
    // Генерируем имя если не указано
    std::string name = textureName;
    if (name.empty()) {
        // Берем имя файла без пути и расширения
        size_t start = filepath.find_last_of("/\\");
        if (start == std::string::npos) start = 0;
        else start++;
        
        size_t end = filepath.find_last_of('.');
        if (end == std::string::npos) end = filepath.length();
        
        name = filepath.substr(start, end - start);
        name = GenerateUniqueName(name);
    }
    
    return CreateTexture2D(name, 0, 0, filepath);
}

Texture* TextureControllerImpl::LoadTextureCube(const std::string& filepath,
                                               const std::string& textureName) {
    Logger::Warning("LoadTextureCube not implemented yet");
    return nullptr;
}

Texture* TextureControllerImpl::GetTexture(const std::string& name) {
    auto it = m_textures.find(name);
    return it != m_textures.end() ? it->second.get() : nullptr;
}

bool TextureControllerImpl::HasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

void TextureControllerImpl::RemoveTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        // Обновляем статистику
        auto* texture = it->second.get();
        if (texture->GetType() == TextureType::Texture2D) {
            m_stats.texture2DCount--;
        } else if (texture->GetType() == TextureType::TextureCube) {
            m_stats.textureCubeCount--;
        }
        
        m_textures.erase(it);
        m_stats.totalTextures--;
        Logger::Info("Texture removed: " + name);
    }
}

Texture* TextureControllerImpl::GetBuiltin(BuiltinTexture type) {
    std::string name = GetBuiltinName(type);
    
    // Проверяем, есть ли уже
    auto* existing = GetTexture(name);
    if (existing) {
        return existing;
    }
    
    // Создаем если нет
    CreateBuiltinTexture(type);
    return GetTexture(name);
}

void TextureControllerImpl::PreloadBuiltinTextures() {
    Logger::Info("Preloading builtin textures...");
    
    // Создаем только основные встроенные текстуры
    CreateBuiltinTexture(BuiltinTexture::White1x1);
    CreateBuiltinTexture(BuiltinTexture::Black1x1);
    CreateBuiltinTexture(BuiltinTexture::Checkerboard);
    
    Logger::Info("Builtin textures preloaded");
}

void TextureControllerImpl::Clear() {
    m_textures.clear();
    m_stats = Statistics();
    Logger::Info("All textures cleared");
}

size_t TextureControllerImpl::GetTextureCount() const {
    return m_textures.size();
}

std::vector<std::string> TextureControllerImpl::GetTextureNames() const {
    std::vector<std::string> names;
    names.reserve(m_textures.size());
    for (const auto& pair : m_textures) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<Texture*> TextureControllerImpl::GetTexturesByType(TextureType type) const {
    std::vector<Texture*> result;
    for (const auto& pair : m_textures) {
        if (pair.second->GetType() == type) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

void TextureControllerImpl::PrintDebugInfo() const {
    Logger::Info("=== TextureControllerImpl Debug Info ===");
    Logger::Info("Total textures: " + std::to_string(m_stats.totalTextures));
    Logger::Info("Texture2D count: " + std::to_string(m_stats.texture2DCount));
    Logger::Info("TextureCube count: " + std::to_string(m_stats.textureCubeCount));
    Logger::Info("Builtin textures: " + std::to_string(m_stats.builtinTextures));
    Logger::Info("Loaded from file: " + std::to_string(m_stats.loadedFromFile));
    Logger::Info("Created procedural: " + std::to_string(m_stats.createdProcedural));
    
    Logger::Info("Loaded textures:");
    for (const auto& pair : m_textures) {
        auto* texture = pair.second.get();
        std::string type = "Unknown";
        if (texture->GetType() == TextureType::Texture2D) type = "Texture2D";
        else if (texture->GetType() == TextureType::TextureCube) type = "TextureCube";
        
        Logger::Info("  - " + pair.first + " (" + type + 
                    ", " + std::to_string(texture->GetWidth()) + 
                    "x" + std::to_string(texture->GetHeight()) + ")");
    }
}

// Вспомогательные методы
std::string TextureControllerImpl::GenerateUniqueName(const std::string& baseName) const {
    std::string name = baseName;
    int counter = 1;
    
    while (HasTexture(name)) {
        std::stringstream ss;
        ss << baseName << "_" << std::setw(3) << std::setfill('0') << counter++;
        name = ss.str();
    }
    
    return name;
}

std::string TextureControllerImpl::GetBuiltinName(BuiltinTexture type) const {
    switch (type) {
        case BuiltinTexture::White1x1: return "Builtin_White1x1";
        case BuiltinTexture::Black1x1: return "Builtin_Black1x1";
        case BuiltinTexture::Gray1x1: return "Builtin_Gray1x1";
        case BuiltinTexture::Red1x1: return "Builtin_Red1x1";
        case BuiltinTexture::Green1x1: return "Builtin_Green1x1";
        case BuiltinTexture::Blue1x1: return "Builtin_Blue1x1";
        case BuiltinTexture::Checkerboard: return "Builtin_Checkerboard";
        case BuiltinTexture::NormalFlat: return "Builtin_NormalFlat";
        case BuiltinTexture::DefaultSkybox: return "Builtin_DefaultSkybox";
        default: return "Builtin_Unknown";
    }
}

void TextureControllerImpl::CreateBuiltinTexture(BuiltinTexture type) {
    std::string name = GetBuiltinName(type);
    
    // Проверяем, не создан ли уже
    if (HasTexture(name)) {
        return;
    }
    
    auto texture = std::make_unique<Texture2D>(name);
    
    switch (type) {
        case BuiltinTexture::White1x1: {
            unsigned char data[] = {255, 255, 255, 255};
            if (auto* tex2D = dynamic_cast<Texture2D*>(texture.get())) {
                tex2D->LoadFromMemory(data, 1, 1, 4);
            }
            break;
        }
        case BuiltinTexture::Black1x1: {
            unsigned char data[] = {0, 0, 0, 255};
            if (auto* tex2D = dynamic_cast<Texture2D*>(texture.get())) {
                tex2D->LoadFromMemory(data, 1, 1, 4);
            }
            break;
        }
        case BuiltinTexture::Checkerboard: {
            unsigned char data[] = {
                255, 255, 255, 255,
                0, 0, 0, 255,
                0, 0, 0, 255,
                255, 255, 255, 255
            };
            if (auto* tex2D = dynamic_cast<Texture2D*>(texture.get())) {
                tex2D->LoadFromMemory(data, 2, 2, 4);
            }
            break;
        }
        default: {
            // Для остальных создаем белый 1x1
            unsigned char data[] = {255, 255, 255, 255};
            if (auto* tex2D = dynamic_cast<Texture2D*>(texture.get())) {
                tex2D->LoadFromMemory(data, 1, 1, 4);
            }
            break;
        }
    }
    
    // Сохраняем
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++;
    m_stats.texture2DCount++;
    m_stats.builtinTextures++;
    
    Logger::Debug("Builtin texture created: " + name);
}

bool TextureControllerImpl::LoadImageFile(const std::string& filepath, 
                                         std::vector<unsigned char>& outData,
                                         int& outWidth, int& outHeight, 
                                         int& outChannels) {
    // TODO: Реализовать через stb_image
    Logger::Warning("LoadImageFile not implemented yet: " + filepath);
    return false;
}

void TextureControllerImpl::WatchTextureFiles(bool enable) {
    m_watchEnabled = enable;
    Logger::Info("Texture file watching " + 
                std::string(enable ? "enabled" : "disabled"));
}

void TextureControllerImpl::CheckForUpdates() {
    if (!m_watchEnabled) return;
    Logger::Warning("Texture hot-reload not implemented yet");
}

} // namespace ogle