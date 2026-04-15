#include "Texture2D.h"
#include "../Logger.h"
#include "../opengl/OpenGLUtils.h" // For GL_CHECK

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // Assuming stb_image is used for image loading

namespace OGLE {

    std::shared_ptr<Texture2D> Texture2D::CreateFromGLuint(GLuint textureId, int width, int height, const std::string& name)
    {
        if (textureId == 0) {
            LOG_ERROR("Attempted to create Texture2D from an invalid texture ID (0).");
            return nullptr;
        }
        // Use `new` to access the private constructor, then wrap in a shared_ptr.
        return std::shared_ptr<Texture2D>(new Texture2D(textureId, width, height, name));
    }

    Texture2D::Texture2D(GLuint textureId, int width, int height, const std::string& name)
        : m_textureID(textureId), m_width(width), m_height(height), m_nrChannels(4), m_filePath(name) {
        // Assumes 4 channels (RGBA32F) for procedural textures.
        LOG_INFO("Texture2D created from existing GLuint: " + name + " (ID: " + std::to_string(textureId) + ")");
    }

    Texture2D::Texture2D()
        : m_textureID(0), m_width(0), m_height(0), m_nrChannels(0) {
    }

    Texture2D::~Texture2D() {
        if (m_textureID != 0) {
            GL_CHECK(glDeleteTextures(1, &m_textureID));
        }
    }

    bool Texture2D::Load(const std::string& filePath) {
        if (m_textureID != 0) { // Already loaded
            LOG_WARN("Texture already loaded: " + m_filePath + ". Skipping reload for: " + filePath);
            return true;
        }

        m_filePath = filePath;
        stbi_set_flip_vertically_on_load(true); // Flip textures on load (OpenGL expects 0,0 at bottom-left)

        unsigned char* data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_nrChannels, 0);
        if (data) {
            GLenum format;
            if (m_nrChannels == 1)
                format = GL_RED;
            else if (m_nrChannels == 3)
                format = GL_RGB;
            else if (m_nrChannels == 4)
                format = GL_RGBA;
            else {
                LOG_ERROR("Unsupported texture format (channels: " + std::to_string(m_nrChannels) + ") for " + filePath);
                stbi_image_free(data);
                return false;
            }

            GL_CHECK(glGenTextures(1, &m_textureID));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_textureID));
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data));
            GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

            stbi_image_free(data);
            LOG_INFO("Texture loaded: " + filePath + " (" + std::to_string(m_width) + "x" + std::to_string(m_height) + ", " + std::to_string(m_nrChannels) + " channels)");
            return true;
        } else {
            LOG_ERROR("Failed to load texture: " + filePath + ". STB_Image error: " + stbi_failure_reason());
            return false;
        }
    }

    void Texture2D::Bind(unsigned int unit) const {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_textureID));
    }

    void Texture2D::Unbind() const {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    }

} // namespace OGLE