// src/render/texture/TextureControllerImpl.h
#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>  // ДОБАВИТЬ ЭТО
#include "log/Logger.h"
#include "Texture.h"

namespace ogle {

// Встроенные текстуры
enum class BuiltinTexture {
    White1x1,
    Black1x1,
    Gray1x1,
    Red1x1,
    Green1x1,
    Blue1x1,
    Checkerboard,
    NormalFlat,
    DefaultSkybox
};

// Приватная реализация
class TextureControllerImpl {
public:
    TextureControllerImpl();
    ~TextureControllerImpl();
    
    // Основные методы
    Texture* CreateTexture2D(const std::string& name, 
                            int width, int height,
                            const std::string& filepath = "");
    
    Texture* CreateTextureCube(const std::string& name,
                              const std::vector<std::string>& facePaths);
    
    Texture* LoadTexture2D(const std::string& filepath, 
                          const std::string& textureName = "");
    
    Texture* LoadTextureCube(const std::string& filepath,
                            const std::string& textureName = "");
    
    Texture* GetTexture(const std::string& name);
    bool HasTexture(const std::string& name) const;
    void RemoveTexture(const std::string& name);
    
    // Builtin текстуры
    Texture* GetBuiltin(BuiltinTexture type);
    void PreloadBuiltinTextures();
    
    // Управление
    void Clear();
    size_t GetTextureCount() const;
    
    std::vector<std::string> GetTextureNames() const;
    std::vector<Texture*> GetTexturesByType(TextureType type) const;
    
    // Отладка
    void PrintDebugInfo() const;
    
    // Hot-reload
    void WatchTextureFiles(bool enable);
    void CheckForUpdates();
    
private:
    // Вспомогательные методы
    std::string GenerateUniqueName(const std::string& baseName) const;
    std::string GetBuiltinName(BuiltinTexture type) const;
    void CreateBuiltinTexture(BuiltinTexture type);
    
    // Загрузка файлов
    bool LoadImageFile(const std::string& filepath, 
                      std::vector<unsigned char>& outData,
                      int& outWidth, int& outHeight, int& outChannels);
    
    // Хранилище
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    
    // Статистика
    struct Statistics {
        size_t totalTextures = 0;
        size_t texture2DCount = 0;
        size_t textureCubeCount = 0;
        size_t builtinTextures = 0;
        size_t loadedFromFile = 0;
        size_t createdProcedural = 0;
    } m_stats;
    
    // Hot-reload
    struct TextureFileInfo {
        std::string filepath;
        std::filesystem::file_time_type lastModified;  // Теперь с правильным пространством имен
    };
    std::unordered_map<std::string, TextureFileInfo> m_fileWatchInfo;
    bool m_watchEnabled = false;
};

} // namespace ogle