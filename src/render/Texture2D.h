#pragma once

#include "../opengl/GLFunctions.h"

#include <memory>
#include <string>

namespace OGLE {
    class Texture2D {
    public:
        ~Texture2D();

        static std::shared_ptr<Texture2D> LoadShared(const std::string& path);

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
