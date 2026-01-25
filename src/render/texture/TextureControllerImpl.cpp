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
    
    bool success = false;
    
    if (!filepath.empty()) {
        // Загружаем из файла
        success = texture->LoadFromFile(filepath);
        if (success) {
            m_stats.loadedFromFile++;
            Logger::Info("Texture loaded from file: " + filepath);
        } else {
            Logger::Error("Failed to load texture from file: " + filepath);
            return nullptr;
        }
    } else {
        // Создаем пустую текстуру
        success = texture->Create(width, height);
        if (success) {
            m_stats.createdProcedural++;
            Logger::Info("Procedural texture created: " + name);
        } else {
            Logger::Error("Failed to create texture: " + name);
            return nullptr;
        }
    }
    
    if (!success) {
        return nullptr;
    }
    
    // Сохраняем
    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++;
    m_stats.texture2DCount++;
    
    // Запоминаем путь для hot-reload
    if (!filepath.empty()) {
        m_fileWatchInfo[name] = {filepath, fs::last_write_time(filepath)};
    }
    
    return ptr;
}

Texture* TextureControllerImpl::CreateTextureCube(const std::string& name,
                                                 const std::vector<std::string>& facePaths) {
    if (facePaths.size() != 6) {
        Logger::Error("Need exactly 6 faces for cube texture");
        return nullptr;
    }
    
    // Проверяем уникальность
    if (HasTexture(name)) {
        Logger::Warning("Texture already exists: " + name);
        return GetTexture(name);
    }
    
    auto texture = std::make_unique<TextureCube>(name);
    
    if (!texture->LoadFromFiles(facePaths[0], facePaths[1], facePaths[2], 
                               facePaths[3], facePaths[4], facePaths[5])) {
        Logger::Error("Failed to load cube texture: " + name);
        return nullptr;
    }
    
    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++;
    m_stats.textureCubeCount++;
    m_stats.loadedFromFile += 6;
    
    Logger::Info("Cube texture created: " + name);
    return ptr;
}

Texture* TextureControllerImpl::LoadTexture2D(const std::string& filepath, 
                                             const std::string& textureName) {
    // Генерируем имя если не указано
    std::string name = textureName;
    if (name.empty()) {
        // Берем имя файла без пути и расширения
        fs::path path(filepath);
        name = path.stem().string();
        name = GenerateUniqueName(name);
    }
    
    return CreateTexture2D(name, 0, 0, filepath);
}

Texture* TextureControllerImpl::LoadTextureCube(const std::string& filepath,
                                               const std::string& textureName) {
    // Для single-file HDR cubemaps
    std::string name = textureName;
    if (name.empty()) {
        fs::path path(filepath);
        name = path.stem().string();
        name = GenerateUniqueName(name + "_cube");
    }
    
    // Проверяем уникальность
    if (HasTexture(name)) {
        return GetTexture(name);
    }
    
    auto texture = std::make_unique<TextureCube>(name);
    
    if (!texture->LoadFromSingleFile(filepath)) {
        Logger::Error("Failed to load HDR cubemap: " + filepath);
        return nullptr;
    }
    
    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++;
    m_stats.textureCubeCount++;
    m_stats.loadedFromFile++;
    
    Logger::Info("HDR cubemap loaded: " + name);
    return ptr;
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
        m_fileWatchInfo.erase(name);
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
    
    // Создаем все встроенные текстуры
    CreateBuiltinTexture(BuiltinTexture::White1x1);
    CreateBuiltinTexture(BuiltinTexture::Black1x1);
    CreateBuiltinTexture(BuiltinTexture::Gray1x1);
    CreateBuiltinTexture(BuiltinTexture::Red1x1);
    CreateBuiltinTexture(BuiltinTexture::Green1x1);
    CreateBuiltinTexture(BuiltinTexture::Blue1x1);
    CreateBuiltinTexture(BuiltinTexture::Checkerboard);
    CreateBuiltinTexture(BuiltinTexture::NormalFlat);
    
    Logger::Info("Builtin textures preloaded");
}

void TextureControllerImpl::Clear() {
    m_textures.clear();
    m_fileWatchInfo.clear();
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
    
    std::unique_ptr<Texture> texture;
    
    switch (type) {
        case BuiltinTexture::White1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {255, 255, 255, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::Black1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {0, 0, 0, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::Gray1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {128, 128, 128, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::Red1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {255, 0, 0, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::Green1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {0, 255, 0, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::Blue1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {0, 0, 255, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::Checkerboard: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {
                255, 255, 255, 255,
                0, 0, 0, 255,
                0, 0, 0, 255,
                255, 255, 255, 255
            };
            tex2d->LoadFromMemory(data, 2, 2, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::NormalFlat: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {128, 128, 255, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
        case BuiltinTexture::DefaultSkybox: {
            auto cubeTexture = std::make_unique<TextureCube>(name);
            cubeTexture->Create(256); // 256x256 каждая грань
            texture = std::move(cubeTexture);
            break;
        }
        default: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {255, 255, 255, 255};
            tex2d->LoadFromMemory(data, 1, 1, 4);
            texture = std::move(tex2d);
            break;
        }
    }
    
    // Сохраняем
    if (texture) {
        m_textures[name] = std::move(texture);
        m_stats.totalTextures++;
        m_stats.builtinTextures++;
        
        // Обновляем статистику по типам
        if (type == BuiltinTexture::DefaultSkybox) {
            m_stats.textureCubeCount++;
        } else {
            m_stats.texture2DCount++;
        }
        
        Logger::Debug("Builtin texture created: " + name);
    }
}


void TextureControllerImpl::WatchTextureFiles(bool enable) {
    m_watchEnabled = enable;
    Logger::Info("Texture file watching " + 
                std::string(enable ? "enabled" : "disabled"));
}

void TextureControllerImpl::CheckForUpdates() {
    if (!m_watchEnabled || m_fileWatchInfo.empty()) return;
    
    bool anyChanged = false;
    
    for (auto& pair : m_fileWatchInfo) {
        const std::string& name = pair.first;
        auto& info = pair.second;
        
        try {
            auto currentTime = fs::last_write_time(info.filepath);
            if (currentTime != info.lastModified) {
                // Файл изменился, перезагружаем текстуру
                auto* texture = GetTexture(name);
                if (texture && texture->GetType() == TextureType::Texture2D) {
                    auto* tex2d = dynamic_cast<Texture2D*>(texture);
                    if (tex2d) {
                        if (tex2d->LoadFromFile(info.filepath)) {
                            info.lastModified = currentTime;
                            anyChanged = true;
                            Logger::Info("Texture hot-reloaded: " + name);
                        }
                    }
                }
            }
        } catch (...) {
            // Файл мог быть удален
            Logger::Warning("Texture file not found: " + info.filepath);
        }
    }
    
    if (anyChanged) {
        Logger::Debug("Texture hot-reload completed");
    }
}

} // namespace ogle