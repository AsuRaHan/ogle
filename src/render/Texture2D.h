#pragma once

#include "../opengl/GLFunctions.h"

#include <memory>
#include <string>

namespace OGLE {
    class Texture2D {
    public:
        ~Texture2D();

        static std::shared_ptr<Texture2D> LoadShared(const std::string& path);
        
        // Создание Texture2D из существующего GLuint (для процедурно-сгенерированных текстур)
        static std::shared_ptr<Texture2D> CreateFromGLuint(GLuint textureId, int width, int height, const std::string& name = "procedural_texture");

        // Load from file without going through the cache (used by TextureManager).
        static std::shared_ptr<Texture2D> LoadFromFileDirect(const std::string& path);

        // Create texture from raw RGBA pixel data (used for fallback/default textures).
        static std::shared_ptr<Texture2D> CreateFromPixels(const unsigned char* pixels, int width, int height, const std::string& name);

        bool IsValid() const { return m_textureId != 0; }
        GLuint GetTextureId() const { return m_textureId; }
        const std::string& GetPath() const { return m_path; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        Texture2D() = default;

        bool LoadFromFile(const std::string& path);

        GLuint m_textureId = 0;
        int m_width = 0;
        int m_height = 0;
        std::string m_path;
    };
}
