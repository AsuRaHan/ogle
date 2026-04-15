#include "TextureManager.h"
#include "../Logger.h"
#include "../core/FileSystem.h"

namespace OGLE {

    TextureManager& TextureManager::Get() {
        static TextureManager instance;
        return instance;
    }

    void TextureManager::Initialize() {
        LOG_INFO("TextureManager initialized.");
        // Any global texture manager setup can go here, e.g., WIC factory if not using stb_image
    }

    std::shared_ptr<Texture2D> TextureManager::GetTexture(const std::string& filePath) {
        if (filePath.empty()) {
            return nullptr;
        }

        // Resolve the path to be absolute and normalized to ensure the cache works correctly
        const std::string resolvedPath = FileSystem::ResolvePath(filePath).string();

        // Check cache first
        auto it = m_textureCache.find(resolvedPath);
        if (it != m_textureCache.end()) {
            return it->second; // Return cached texture (even if it's nullptr for a failed load)
        }

        // Not in cache, try to load it
        LOG_INFO("Loading new texture: " + resolvedPath);
        auto texture = std::make_shared<Texture2D>();
        if (texture->Load(resolvedPath)) {
            m_textureCache[resolvedPath] = texture;
            return texture;
        }

        // Cache the failure so we don't try to load it again
        m_textureCache[resolvedPath] = nullptr;
        return nullptr;
    }

} // namespace OGLE