#pragma once

#include <string>
#include <map>
#include <memory>
#include "Texture2D.h"

namespace OGLE {

    // Manages loading and caching of Texture2D objects.
    class TextureManager {
    public:
        static TextureManager& Get(); // Singleton access

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        // Initializes the texture manager (e.g., sets up WIC factory)
        void Initialize();

        // Loads a texture from file or returns it from cache if already loaded.
        std::shared_ptr<Texture2D> GetTexture(const std::string& filePath);

    private:
        TextureManager() = default; // Private constructor for singleton

        std::map<std::string, std::shared_ptr<Texture2D>> m_textureCache;
    };

} // namespace OGLE