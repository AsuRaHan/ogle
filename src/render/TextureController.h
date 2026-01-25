// src/render/TextureController.h
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "log/Logger.h"

namespace ogle {

// Предварительные объявления
class Texture;
class TextureControllerImpl;
enum class TextureType;

// Публичный интерфейс (синглтон)
class TextureController {
public:
    static TextureController& Get();
    
    ~TextureController();
    
    // === Создание текстур ===
    Texture* CreateTexture2D(const std::string& name, 
                            int width, int height,
                            const std::string& filepath = "");
    
    Texture* CreateTextureCube(const std::string& name,
                              const std::vector<std::string>& facePaths);
    
    // === Загрузка из файлов ===
    Texture* LoadTexture2D(const std::string& filepath, 
                          const std::string& textureName = "");
    
    Texture* LoadTextureCube(const std::string& filepath,
                            const std::string& textureName = "");
    
    // === Управление текстурами ===
    Texture* GetTexture(const std::string& name);
    bool HasTexture(const std::string& name) const;
    void RemoveTexture(const std::string& name);
    
    // === Встроенные текстуры ===
    enum class Builtin {
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
    
    Texture* GetBuiltin(Builtin type);
    void PreloadBuiltinTextures();
    
    // === Утилиты ===
    void Clear();
    size_t GetTextureCount() const;
    
    std::vector<std::string> GetTextureNames() const;
    std::vector<Texture*> GetTexturesByType(TextureType type) const;
    
    // === Отладка ===
    void PrintDebugInfo() const;
    
    // === Hot-reload (опционально) ===
    void WatchTextureFiles(bool enable);
    void CheckForUpdates();
    
private:
    TextureController();
    
    std::unique_ptr<TextureControllerImpl> m_impl;
    
    // Запрет копирования
    TextureController(const TextureController&) = delete;
    TextureController& operator=(const TextureController&) = delete;
};

} // namespace ogle