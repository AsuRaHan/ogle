#pragma once

#include "render/Texture2D.h"
#include "core/FileSystem.h"

#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace OGLE {
// ─────────────────────────────────────────────────────────────────────────────
// TextureManager — centralized texture cache with stats, fallback, and reload.
//
// Owns the authoritative cache of all loaded textures. Texture2D::LoadShared()
// delegates here. Use this class to query, enumerate, reload, or unload textures.
//
// Singleton: TextureManager::Get()
// ─────────────────────────────────────────────────────────────────────────────

class TextureManager
{
public:
    struct TextureInfo
    {
        std::string path;
        int width = 0;
        int height = 0;
        size_t estimatedBytes = 0;  // RGBA8 = width * height * 4
        bool isValid = false;
    };

    // ── Singleton ──────────────────────────────────────────────────────────
    static TextureManager& Get()
    {
        static TextureManager instance;
        return instance;
    }

    // ── Lifecycle ──────────────────────────────────────────────────────────
    // Call Initialize() once during app startup (after OpenGL context is ready).
    // Generates the fallback and default-white textures.
    bool Initialize()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_initialized)
            return true;

        // Generate 2x2 magenta fallback texture
        {
            unsigned char pixels[2 * 2 * 4];
            for (int i = 0; i < 16; i += 4)
            {
                pixels[i + 0] = 255; // R
                pixels[i + 1] = 0;   // G
                pixels[i + 2] = 255; // B
                pixels[i + 3] = 255; // A
            }
            m_fallbackTexture = Texture2D::CreateFromPixels(pixels, 2, 2, "__fallback_magenta__");
        }

        // Generate 1x1 white default texture
        {
            unsigned char pixels[1 * 1 * 4] = { 255, 255, 255, 255 };
            m_defaultWhiteTexture = Texture2D::CreateFromPixels(pixels, 1, 1, "__default_white__");
        }

        m_initialized = true;
        return true;
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_cache.clear();
        m_fallbackTexture.reset();
        m_defaultWhiteTexture.reset();
        m_initialized = false;
    }

    // ── Load ───────────────────────────────────────────────────────────────
    // Load a texture from file. Returns cached version if already loaded.
    // If the file does not exist or fails to load, returns the fallback texture.
    std::shared_ptr<Texture2D> Load(const std::string& path)
    {
        if (path.empty())
            return m_defaultWhiteTexture;

        const std::string resolved = ResolvePath(path);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(resolved);
            if (it != m_cache.end())
                return it->second;
        }

        std::shared_ptr<Texture2D> texture = Texture2D::LoadFromFileDirect(resolved);
        if (!texture || !texture->IsValid())
        {
            return m_fallbackTexture;
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cache[resolved] = texture;
        }

        return texture;
    }

    // ── Reload ─────────────────────────────────────────────────────────────
    // Reloads a texture from disk. Updates the cache entry.
    // Returns true if reload succeeded, false if the file could not be reloaded.
    bool Reload(const std::string& path)
    {
        if (path.empty())
            return false;

        const std::string resolved = ResolvePath(path);

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(resolved);
        if (it == m_cache.end())
        {
            // Not cached — try loading it fresh
            auto texture = Texture2D::LoadFromFileDirect(resolved);
            if (texture && texture->IsValid())
            {
                m_cache[resolved] = texture;
                return true;
            }
            return false;
        }

        // The old shared_ptr will be destroyed when replaced,
        // which calls ~Texture2D() and frees the old GLuint.
        auto texture = Texture2D::LoadFromFileDirect(resolved);
        if (texture && texture->IsValid())
        {
            it->second = texture;
            return true;
        }

        return false;
    }

    // ── Query ──────────────────────────────────────────────────────────────
    bool HasTexture(const std::string& path) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.find(ResolvePath(path)) != m_cache.end();
    }

    std::shared_ptr<Texture2D> GetTexture(const std::string& path) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(ResolvePath(path));
        if (it != m_cache.end())
            return it->second;
        return nullptr;
    }

    TextureInfo GetInfo(const std::string& path) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(ResolvePath(path));
        if (it == m_cache.end() || !it->second)
            return TextureInfo{};

        const auto& tex = it->second;
        return TextureInfo{
            tex->GetPath(),
            tex->GetWidth(),
            tex->GetHeight(),
            static_cast<size_t>(tex->GetWidth()) * static_cast<size_t>(tex->GetHeight()) * 4,
            tex->IsValid()
        };
    }

    // ── Enumerate ──────────────────────────────────────────────────────────
    std::vector<TextureInfo> GetAllLoadedTextures() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<TextureInfo> result;
        result.reserve(m_cache.size());

        for (const auto& pair : m_cache)
        {
            const auto& tex = pair.second;
            if (tex && tex->IsValid())
            {
                result.push_back(TextureInfo{
                    tex->GetPath(),
                    tex->GetWidth(),
                    tex->GetHeight(),
                    static_cast<size_t>(tex->GetWidth()) * static_cast<size_t>(tex->GetHeight()) * 4,
                    tex->IsValid()
                });
            }
        }

        return result;
    }

    std::vector<std::string> GetLoadedPaths() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> result;
        result.reserve(m_cache.size());
        for (const auto& pair : m_cache)
        {
            if (pair.second && pair.second->IsValid())
                result.push_back(pair.first);
        }
        return result;
    }

    // ── Stats ──────────────────────────────────────────────────────────────
    size_t GetLoadedCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t count = 0;
        for (const auto& pair : m_cache)
        {
            if (pair.second && pair.second->IsValid())
                ++count;
        }
        return count;
    }

    size_t GetEstimatedVRAMBytes() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t total = 0;
        for (const auto& pair : m_cache)
        {
            if (pair.second && pair.second->IsValid())
            {
                total += static_cast<size_t>(pair.second->GetWidth())
                       * static_cast<size_t>(pair.second->GetHeight()) * 4;
            }
        }
        // Add mipmaps overhead (approximately 1.33x for full mip chain)
        return total + total / 3;
    }

    std::string GetEstimatedVRAMString() const
    {
        const size_t bytes = GetEstimatedVRAMBytes();
        if (bytes < 1024)
            return std::to_string(bytes) + " B";
        if (bytes < 1024 * 1024)
            return std::to_string(bytes / 1024) + " KB";
        if (bytes < 1024 * 1024 * 1024)
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        return std::to_string(bytes / (1024 * 1024 * 1024)) + " GB";
    }

    // ── Unload ─────────────────────────────────────────────────────────────
    void Unload(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.erase(ResolvePath(path));
    }

    void UnloadAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.clear();
    }

    // ── Fallbacks ──────────────────────────────────────────────────────────
    std::shared_ptr<Texture2D> GetFallbackTexture() const { return m_fallbackTexture; }
    std::shared_ptr<Texture2D> GetDefaultWhiteTexture() const { return m_defaultWhiteTexture; }

    bool IsFallbackTexture(const std::shared_ptr<Texture2D>& texture) const
    {
        return texture && texture->GetPath() == "__fallback_magenta__";
    }

    bool IsDefaultWhiteTexture(const std::shared_ptr<Texture2D>& texture) const
    {
        return texture && texture->GetPath() == "__default_white__";
    }

private:
    TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    static std::string ResolvePath(const std::string& path)
    {
        // Forward to the engine's FileSystem.
        // We include FileSystem.h in TextureManager.h for this.
        return FileSystem::ResolvePath(path).string();
    }

    bool m_initialized = false;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_cache;
    std::shared_ptr<Texture2D> m_fallbackTexture;
    std::shared_ptr<Texture2D> m_defaultWhiteTexture;
};

} // namespace OGLE
