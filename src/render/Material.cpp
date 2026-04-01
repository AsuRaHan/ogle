#include "render/Material.h"

#include "Logger.h"

namespace OGLE {
    void Material::Bind(GLuint program) const
    {
        const GLint baseColorLocation = glGetUniformLocation(program, "uBaseColor");
        if (baseColorLocation >= 0) {
            glUniform3f(baseColorLocation, m_baseColor.x, m_baseColor.y, m_baseColor.z);
        }

        const GLint hasTextureLocation = glGetUniformLocation(program, "uHasDiffuseTexture");
        const GLint textureSamplerLocation = glGetUniformLocation(program, "uDiffuseTexture");

        glActiveTexture(GL_TEXTURE0);
        if (m_diffuseTexture && m_diffuseTexture->IsValid()) {
            glBindTexture(GL_TEXTURE_2D, m_diffuseTexture->GetTextureId());
            if (hasTextureLocation >= 0) {
                glUniform1i(hasTextureLocation, 1);
            }
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
            if (hasTextureLocation >= 0) {
                glUniform1i(hasTextureLocation, 0);
            }
        }

        if (textureSamplerLocation >= 0) {
            glUniform1i(textureSamplerLocation, 0);
        }
    }

    void Material::SetBaseColor(const glm::vec3& color)
    {
        m_baseColor = color;
    }

    const glm::vec3& Material::GetBaseColor() const
    {
        return m_baseColor;
    }

    bool Material::SetDiffuseTexturePath(const std::string& path)
    {
        if (path.empty()) {
            ClearDiffuseTexture();
            return true;
        }

        std::shared_ptr<Texture2D> texture = Texture2D::LoadShared(path);
        if (!texture || !texture->IsValid()) {
            LOG_WARN("Failed to load diffuse texture: " + path);
            return false;
        }

        m_diffuseTexture = std::move(texture);
        m_diffuseTexturePath = m_diffuseTexture->GetPath();
        return true;
    }

    void Material::ClearDiffuseTexture()
    {
        m_diffuseTexture.reset();
        m_diffuseTexturePath.clear();
    }

    bool Material::HasDiffuseTexture() const
    {
        return m_diffuseTexture && m_diffuseTexture->IsValid();
    }

    const std::string& Material::GetDiffuseTexturePath() const
    {
        return m_diffuseTexturePath;
    }
}
