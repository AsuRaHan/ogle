#pragma once

#include <string>
#include <memory> // For std::shared_ptr
#include "../opengl/GLFunctions.h" // Используем ручную загрузку функций

namespace OGLE {

    // Represents an OpenGL 2D texture.
    class Texture2D {
    public:
        // Factory method for creating a Texture2D from an existing OpenGL texture ID.
        // This is useful for procedural textures or framebuffer attachments.
        static std::shared_ptr<Texture2D> CreateFromGLuint(GLuint textureId, int width, int height, const std::string& name);

        Texture2D();
        ~Texture2D();

        // Loads a texture from a file.
        // This should primarily be called by TextureManager.
        bool Load(const std::string& filePath);

        // Binds the texture to a specific texture unit.
        void Bind(unsigned int unit = 0) const;

        // Unbinds the texture.
        void Unbind() const;

        // Checks if the texture has a valid OpenGL ID.
        bool IsValid() const { return m_textureID != 0; }

        GLuint GetTextureId() const { return m_textureID; }
        const std::string& GetPath() const { return m_filePath; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        // Private constructor for use by the static factory method.
        Texture2D(GLuint textureId, int width, int height, const std::string& name);

        GLuint m_textureID;
        std::string m_filePath; // Can be a file path or a generated name like "procedural_clouds"
        int m_width, m_height, m_nrChannels;
    };

} // namespace OGLE