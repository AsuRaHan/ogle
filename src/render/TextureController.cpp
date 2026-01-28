// src/render/TextureController.cpp
#include "TextureController.h"
#include "render/texture/Texture.h"

namespace fs = std::filesystem;

namespace ogle {

TextureController& TextureController::Get() {
    static TextureController instance;
    return instance;
}

TextureController::TextureController() {
    Logger::Info("TextureController initialized");
    PreloadBuiltinTextures();
}

TextureController::~TextureController() {
    Clear();
    Logger::Info("TextureController shutdown");
}

Texture* TextureController::CreateTexture2D(const std::string& name,
                                           int width, int height,
                                           const std::string& filepath) {
    if (HasTexture(name)) { Logger::Warning("Texture already exists: " + name); return GetTexture(name); }

    auto texture = std::make_unique<Texture2D>(name);
    bool success = false;

    if (!filepath.empty()) {
        success = texture->LoadFromFile(filepath);
        if (success) { m_stats.loadedFromFile++; Logger::Info("Texture loaded from file: " + filepath); }
        else { Logger::Error("Failed to load texture from file: " + filepath); return nullptr; }
    } else {
        success = texture->Create(width, height);
        if (success) { m_stats.createdProcedural++; Logger::Info("Procedural texture created: " + name); }
        else { Logger::Error("Failed to create texture: " + name); return nullptr; }
    }

    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++; m_stats.texture2DCount++;

    if (!filepath.empty()) {
        try { m_fileWatchInfo[name] = { filepath, fs::last_write_time(filepath) }; } catch (...) {}
    }

    return ptr;
}

Texture* TextureController::CreateTextureCube(const std::string& name, const std::vector<std::string>& facePaths) {
    if (facePaths.size() != 6) { Logger::Error("Need exactly 6 faces for cube texture"); return nullptr; }
    if (HasTexture(name)) { Logger::Warning("Texture already exists: " + name); return GetTexture(name); }

    auto texture = std::make_unique<TextureCube>(name);
    if (!texture->LoadFromFiles(facePaths[0], facePaths[1], facePaths[2], facePaths[3], facePaths[4], facePaths[5])) { Logger::Error("Failed to load cube texture: " + name); return nullptr; }

    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++; m_stats.textureCubeCount++; m_stats.loadedFromFile += 6;
    Logger::Info("Cube texture created: " + name);
    return ptr;
}

Texture* TextureController::LoadTexture2D(const std::string& filepath, const std::string& textureName) {
    std::string name = textureName;
    if (name.empty()) { fs::path path(filepath); name = path.stem().string(); name = GenerateUniqueName(name); }
    return CreateTexture2D(name, 0, 0, filepath);
}

Texture* TextureController::LoadTextureCube(const std::string& filepath, const std::string& textureName) {
    std::string name = textureName;
    if (name.empty()) { fs::path path(filepath); name = path.stem().string(); name = GenerateUniqueName(name + "_cube"); }
    if (HasTexture(name)) return GetTexture(name);

    auto texture = std::make_unique<TextureCube>(name);
    if (!texture->LoadFromSingleFile(filepath)) { Logger::Error("Failed to load HDR cubemap: " + filepath); return nullptr; }

    Texture* ptr = texture.get();
    m_textures[name] = std::move(texture);
    m_stats.totalTextures++; m_stats.textureCubeCount++; m_stats.loadedFromFile++;
    Logger::Info("HDR cubemap loaded: " + name);
    return ptr;
}

Texture* TextureController::GetTexture(const std::string& name) {
    auto it = m_textures.find(name);
    return it != m_textures.end() ? it->second.get() : nullptr;
}

bool TextureController::HasTexture(const std::string& name) const { return m_textures.find(name) != m_textures.end(); }

void TextureController::RemoveTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        auto* texture = it->second.get();
        if (texture->GetType() == TextureType::Texture2D) m_stats.texture2DCount--; else if (texture->GetType() == TextureType::TextureCube) m_stats.textureCubeCount--;
        m_textures.erase(it);
        m_stats.totalTextures--;
        m_fileWatchInfo.erase(name);
        Logger::Info("Texture removed: " + name);
    }
}

Texture* TextureController::GetBuiltin(Builtin type) {
    std::string name = GetBuiltinName(static_cast<BuiltinTexture>(type));
    auto* existing = GetTexture(name);
    if (existing) return existing;
    CreateBuiltinTexture(static_cast<BuiltinTexture>(type));
    return GetTexture(name);
}

void TextureController::PreloadBuiltinTextures() {
    Logger::Info("Preloading builtin textures...");
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

void TextureController::Clear() {
    m_textures.clear(); m_fileWatchInfo.clear(); m_stats = Statistics(); Logger::Info("All textures cleared");
}

size_t TextureController::GetTextureCount() const { return m_textures.size(); }

std::vector<std::string> TextureController::GetTextureNames() const { std::vector<std::string> names; names.reserve(m_textures.size()); for (const auto& pair : m_textures) names.push_back(pair.first); return names; }

std::vector<Texture*> TextureController::GetTexturesByType(TextureType type) const { std::vector<Texture*> result; for (const auto& pair : m_textures) if (pair.second->GetType() == type) result.push_back(pair.second.get()); return result; }

void TextureController::PrintDebugInfo() const {
    Logger::Info("=== TextureController Debug Info ===");
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
        if (texture->GetType() == TextureType::Texture2D) type = "Texture2D"; else if (texture->GetType() == TextureType::TextureCube) type = "TextureCube";
        Logger::Info("  - " + pair.first + " (" + type + ", " + std::to_string(texture->GetWidth()) + "x" + std::to_string(texture->GetHeight()) + ")");
    }
}

std::string TextureController::GenerateUniqueName(const std::string& baseName) const {
    std::string name = baseName; int counter = 1; while (HasTexture(name)) { std::stringstream ss; ss << baseName << "_" << std::setw(3) << std::setfill('0') << counter++; name = ss.str(); } return name;
}

std::string TextureController::GetBuiltinName(BuiltinTexture type) const {
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

void TextureController::CreateBuiltinTexture(BuiltinTexture type) {
    std::string name = GetBuiltinName(type);
    if (HasTexture(name)) return;
    std::unique_ptr<Texture> texture;
    switch (type) {
        case BuiltinTexture::White1x1: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {255,255,255,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::Black1x1: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {0,0,0,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::Gray1x1: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {128,128,128,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::Red1x1: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {255,0,0,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::Green1x1: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {0,255,0,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::Blue1x1: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {0,0,255,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::Checkerboard: {
            auto tex2d = std::make_unique<Texture2D>(name);
            unsigned char data[] = {255,255,255,255, 0,0,0,255, 0,0,0,255, 255,255,255,255};
            tex2d->LoadFromMemory(data,2,2,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::NormalFlat: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {128,128,255,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
        case BuiltinTexture::DefaultSkybox: {
            auto cubeTexture = std::make_unique<TextureCube>(name); cubeTexture->Create(256); texture = std::move(cubeTexture); break;
        }
        default: {
            auto tex2d = std::make_unique<Texture2D>(name); unsigned char data[] = {255,255,255,255}; tex2d->LoadFromMemory(data,1,1,4); texture = std::move(tex2d); break;
        }
    }

    if (texture) {
        m_textures[name] = std::move(texture);
        m_stats.totalTextures++; m_stats.builtinTextures++;
        if (type == BuiltinTexture::DefaultSkybox) m_stats.textureCubeCount++; else m_stats.texture2DCount++;
        Logger::Debug("Builtin texture created: " + name);
    }
}

void TextureController::WatchTextureFiles(bool enable) { m_watchEnabled = enable; Logger::Info("Texture file watching " + std::string(enable ? "enabled" : "disabled")); }

void TextureController::CheckForUpdates() {
    if (!m_watchEnabled || m_fileWatchInfo.empty()) return;
    bool anyChanged = false;
    for (auto& pair : m_fileWatchInfo) {
        const std::string& name = pair.first; auto& info = pair.second;
        try {
            auto currentTime = fs::last_write_time(info.filepath);
            if (currentTime != info.lastModified) {
                auto* texture = GetTexture(name);
                if (texture && texture->GetType() == TextureType::Texture2D) {
                    auto* tex2d = dynamic_cast<Texture2D*>(texture);
                    if (tex2d) {
                        if (tex2d->LoadFromFile(info.filepath)) { info.lastModified = currentTime; anyChanged = true; Logger::Info("Texture hot-reloaded: " + name); }
                    }
                }
            }
        } catch (...) { Logger::Warning("Texture file not found: " + info.filepath); }
    }
    if (anyChanged) Logger::Debug("Texture hot-reload completed");
}

} // namespace ogle
