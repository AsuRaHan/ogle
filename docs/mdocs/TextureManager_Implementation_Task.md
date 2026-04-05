# TASK: Implement TextureManager

## Overview
Your engine already has a texture cache inside `Texture2D.cpp` (anonymous namespace, `weak_ptr` based). It works, but it is invisible — nothing can query it, nothing can enumerate loaded textures, no fallback for missing files, no hot reload, no stats.

You need to build a **TextureManager** that:
1. Takes ownership of the texture cache (visible, enumerable, manageable)
2. Generates a **fallback texture** (magenta 2x2) for missing files — instead of showing black
3. Generates a **default white texture** for materials that have no texture path
4. Provides **VRAM estimation** and **loaded texture count**
5. Provides **hot reload** — reload a texture from disk without restarting
6. Provides **explicit unload** — force unload a texture even if referenced
7. Integrates cleanly into `App` as a member

**This task creates 1 NEW file and modifies 4 EXISTING files.**

---

## Files to Create / Modify

| # | File | Action | What |
|---|------|--------|------|
| 1 | `render/TextureManager.h` | **CREATE** | TextureManager class (header-only) |
| 2 | `render/Texture2D.h` | **MODIFY** | Add `CreateFromPixels()` static method |
| 3 | `render/Texture2D.cpp` | **MODIFY** | Delegate to TextureManager, add CreateFromPixels |
| 4 | `App.h` | **MODIFY** | Add TextureManager include and member |
| 5 | `App.cpp` | **MODIFY** | Initialize TextureManager, add to main loop |

---

## STEP 1: Create `render/TextureManager.h`

Create this file. It is **header-only** (no .cpp needed).

**Full content:**

```cpp
#pragma once

#include "render/Texture2D.h"

#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

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
        // Use the engine's FileSystem to resolve relative paths.
        // If FileSystem is not available, return path as-is.
        // The include for FileSystem is in Texture2D.cpp — we keep it simple here.
        return path;
    }

    bool m_initialized = false;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_cache;
    std::shared_ptr<Texture2D> m_fallbackTexture;
    std::shared_ptr<Texture2D> m_defaultWhiteTexture;
};
```

---

## STEP 2: Modify `render/Texture2D.h`

Add **two new public static methods** to the `Texture2D` class.

Find this line in the public section:
```cpp
        static std::shared_ptr<Texture2D> CreateFromGLuint(GLuint textureId, int width, int height, const std::string& name = "procedural_texture");
```

**Add these two lines AFTER it:**

```cpp
        // Load from file without going through the cache (used by TextureManager).
        static std::shared_ptr<Texture2D> LoadFromFileDirect(const std::string& path);

        // Create texture from raw RGBA pixel data (used for fallback/default textures).
        static std::shared_ptr<Texture2D> CreateFromPixels(const unsigned char* pixels, int width, int height, const std::string& name);
```

After the change, the public section should look like this:
```cpp
    public:
        ~Texture2D();

        static std::shared_ptr<Texture2D> LoadShared(const std::string& path);

        static std::shared_ptr<Texture2D> CreateFromGLuint(GLuint textureId, int width, int height, const std::string& name = "procedural_texture");

        // Load from file without going through the cache (used by TextureManager).
        static std::shared_ptr<Texture2D> LoadFromFileDirect(const std::string& path);

        // Create texture from raw RGBA pixel data (used for fallback/default textures).
        static std::shared_ptr<Texture2D> CreateFromPixels(const unsigned char* pixels, int width, int height, const std::string& name);

        bool IsValid() const { return m_textureId != 0; }
        // ... rest unchanged ...
```

**DO NOT change anything else in Texture2D.h.**

---

## STEP 3: Modify `render/Texture2D.cpp`

### 3a. Add include at top

Add this include near the top of the file (with the other includes):
```cpp
#include "render/TextureManager.h"
```

### 3b. Modify `LoadShared()` to delegate to TextureManager

Find this entire function:
```cpp
    std::shared_ptr<Texture2D> Texture2D::LoadShared(const std::string& path)
    {
        const std::string resolvedPath = FileSystem::ResolvePath(path).string();

        {
            std::lock_guard<std::mutex> lock(g_textureCacheMutex);
            const auto cacheIt = g_textureCache.find(resolvedPath);
            if (cacheIt != g_textureCache.end()) {
                if (std::shared_ptr<Texture2D> cachedTexture = cacheIt->second.lock()) {
                    return cachedTexture;
                }
            }
        }

        std::shared_ptr<Texture2D> texture(new Texture2D());
        if (!texture->LoadFromFile(resolvedPath)) {
            return {};
        }

        {
            std::lock_guard<std::mutex> lock(g_textureCacheMutex);
            g_textureCache[resolvedPath] = texture;
        }

        return texture;
    }
```

**Replace the ENTIRE function with this:**

```cpp
    std::shared_ptr<Texture2D> Texture2D::LoadShared(const std::string& path)
    {
        return TextureManager::Get().Load(path);
    }
```

**Yes, it is now just one line.** All the caching logic is in TextureManager now.

### 3c. Add `LoadFromFileDirect()` method

Add this method right after the new `LoadShared()`:

```cpp
    std::shared_ptr<Texture2D> Texture2D::LoadFromFileDirect(const std::string& path)
    {
        std::shared_ptr<Texture2D> texture(new Texture2D());
        if (!texture->LoadFromFile(path))
        {
            return {};
        }
        return texture;
    }
```

### 3d. Add `CreateFromPixels()` method

Add this method after `CreateFromGLuint()`:

```cpp
    std::shared_ptr<Texture2D> Texture2D::CreateFromPixels(const unsigned char* pixels, int width, int height, const std::string& name)
    {
        if (!pixels || width <= 0 || height <= 0)
        {
            LOG_ERROR("Invalid parameters for CreateFromPixels: width=" + std::to_string(width) + ", height=" + std::to_string(height));
            return nullptr;
        }

        std::shared_ptr<Texture2D> texture(new Texture2D());

        glGenTextures(1, &texture->m_textureId);
        glBindTexture(GL_TEXTURE_2D, texture->m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        texture->m_width = width;
        texture->m_height = height;
        texture->m_path = name;

        return texture;
    }
```

### 3e. Remove the old cache variables

Find and **DELETE** these lines from the anonymous namespace at the top of the file:

```cpp
        std::mutex g_textureCacheMutex;
        std::unordered_map<std::string, std::weak_ptr<Texture2D>> g_textureCache;
```

The anonymous namespace still keeps `EnsureComInitialized()` — leave that function as-is.

### 3f. Fix TextureManager::ResolvePath

The `ResolvePath()` in TextureManager.h is a stub (returns path as-is). We need it to actually resolve paths. Open `TextureManager.h` and find the `ResolvePath()` method at the bottom:

```cpp
    static std::string ResolvePath(const std::string& path)
    {
        // Use the engine's FileSystem to resolve relative paths.
        // If FileSystem is not available, return path as-is.
        // The include for FileSystem is in Texture2D.cpp — we keep it simple here.
        return path;
    }
```

**Replace it with:**

```cpp
    static std::string ResolvePath(const std::string& path)
    {
        // Forward to the engine's FileSystem.
        // We include FileSystem.h in TextureManager.h for this.
        return FileSystem::ResolvePath(path).string();
    }
```

**And add this include at the top of `TextureManager.h`:**

```cpp
#include "core/FileSystem.h"
```

**IMPORTANT:** Check if `FileSystem::ResolvePath()` returns a `std::filesystem::path`. If it does, `.string()` converts it. If your version returns `std::string` directly, remove `.string()` and just use `return FileSystem::ResolvePath(path);`. Look at how it is used in the existing `Texture2D::LoadShared()` code — it uses `.string()`, so follow the same pattern.

---

## STEP 4: Modify `App.h`

### 4a. Add include

Add this include near the top of the file (with the other includes):
```cpp
#include "render/TextureManager.h"
```

### 4b. Add getter

Add this getter in the public section, after the other getters (after `GetWorldManager`):

```cpp
    TextureManager& GetTextureManager() { return m_textureManager; }
    const TextureManager& GetTextureManager() const { return m_textureManager; }
```

### 4c. Add member

Add this in the private section, after the other member variables:

```cpp
    TextureManager m_textureManager;
```

---

## STEP 5: Modify `App.cpp`

### 5a. Add include (if not already present from previous tasks)

```cpp
#include "render/TextureManager.h"
```

### 5b. Initialize TextureManager

In `App::Run()`, find this line:
```cpp
    m_window->Show(nCmdShow);
```

**Add this line BEFORE `m_window->Show()`:**

```cpp
    m_textureManager.Initialize();
```

### 5c. Add hot-reload key binding (optional but recommended)

In the main loop, find this line:
```cpp
        const float deltaTime = m_timeManager.Tick();
```

**Add this block right after it** (before `m_inputManager.Update(deltaTime);`):

```cpp
        // Hot-reload textures: press F5 to reload all textures from disk
        // (This is a convenience feature during development)
```

**NOTE:** Actually, skip the F5 hot-reload for now. That should be a separate task with input binding. Just the TextureManager itself is enough.

---

## STEP 6: Verify the flow works

After all changes, the texture loading flow is:

```
Material::SetDiffuseTexturePath("assets/textures/wood.png")
  └─► Texture2D::LoadShared(path)
        └─► TextureManager::Get().Load(path)
              ├─► Cache hit? Return cached shared_ptr
              └─► Cache miss?
                    ├─► Texture2D::LoadFromFileDirect(path)
                    │     └─► LoadFromFile() using WIC → glGenTextures → return texture
                    ├─► Success? Store in cache, return
                    └─► Failed? Return fallback magenta texture
```

When a texture fails to load (file not found, corrupt file, etc.):
- **Before:** Material had null `m_diffuseTexture`, `Bind()` set `uHasDiffuseTexture = 0`, object rendered with base color only (no visible error)
- **After:** Material gets magenta fallback texture, `Bind()` sets `uHasDiffuseTexture = 1`, object renders with magenta — clearly visible that texture is missing

---

## Summary of ALL Changes

| File | Change | Lines |
|------|--------|-------|
| `render/TextureManager.h` | **NEW** — 220 lines, header-only | Full file |
| `render/Texture2D.h` | Add 2 method declarations | +4 lines |
| `render/Texture2D.cpp` | Delegate LoadShared to TextureManager, add LoadFromFileDirect + CreateFromPixels, remove old cache | ~30 lines changed |
| `App.h` | Add include, getter, member | +4 lines |
| `App.cpp` | Add include, initialize TextureManager | +2 lines |

**NO other files are changed.** Material.h, Material.cpp, OpenGLRenderer, WorldManager, Editor — all unchanged.

---

## What Improvements This Brings

| Before | After |
|--------|-------|
| Missing texture → black object (invisible bug) | Missing texture → magenta object (obvious visual indicator) |
| Cache hidden in anonymous namespace | Cache accessible via `TextureManager::Get()` |
| Cannot query loaded textures | `GetAllLoadedTextures()`, `GetLoadedPaths()` |
| Cannot estimate VRAM usage | `GetEstimatedVRAMString()` returns "12.5 MB" |
| Cannot reload texture from disk | `Reload("path")` reloads from file |
| Cannot unload specific texture | `Unload("path")` frees GPU memory |
| `LoadShared()` returned null on failure | Now returns fallback magenta texture |
| No default white texture | `GetDefaultWhiteTexture()` for empty material slots |

---

## Build and Test

```
PS E:\my_proj\ogle> .\build.bat Release
```

**Expected result:**
- Compiles without errors.
- Engine runs normally — all existing textures still load and display correctly.
- Existing worlds load without issues (Material::FromJson still calls SetDiffuseTexturePath which still calls LoadShared which now goes through TextureManager).
- Breaking a texture path (deleting the file) shows magenta instead of black.

**If you get compile errors:**

1. **`CreateFromPixels` not declared:** Check that you added the declaration in `Texture2D.h` public section.
2. **`LoadFromFileDirect` not declared:** Same — check `Texture2D.h`.
3. **`FileSystem::ResolvePath` not found:** Check the include path. If `core/FileSystem.h` doesn't exist at that path, find where `FileSystem` is declared and adjust the include. The existing `Texture2D.cpp` already includes `"core/FileSystem.h"` — use the same include path.
4. **`TextureManager` not found:** Check that `render/TextureManager.h` is included in `App.h` and `Texture2D.cpp`.
5. **Linker errors:** Make sure `TextureManager.h` is included in at least one .cpp file that gets compiled (App.cpp is sufficient since it includes App.h which includes TextureManager.h).

---

## Testing Checklist

After building, verify these scenarios:

1. **Normal texture loading:** Load a world with textured objects. Everything looks the same as before.
2. **Missing texture:** Change a texture path in the inspector to a nonexistent file. Object should show magenta instead of black.
3. **Empty texture path:** Clear a texture path. Object should show with base color only (no crash).
4. **Same texture on multiple objects:** Create 5 cubes all using `"assets/textures/wood.png"`. Check that `TextureManager::Get().GetLoadedCount()` returns 1 (not 5). The texture is shared.
5. **VRAM stats:** Call `TextureManager::Get().GetEstimatedVRAMString()` — should return a human-readable string like "2.5 MB".

---

## Future Improvements (DO NOT DO THESE NOW)

These are ideas for later tasks:
- **F5 hot-reload:** Bind a key to `TextureManager::Get().ReloadAll()` to reload all textures during development.
- **sRGB handling:** Detect sRGB textures and set `GL_SRGB8_ALPHA8` internal format.
- **Texture compression:** Support DDS/KTX compressed formats for lower VRAM.
- **Texture streaming:** Load mip levels progressively for large textures.
- **Editor panel:** Add a "Textures" panel in the editor showing all loaded textures, their sizes, and VRAM usage.
