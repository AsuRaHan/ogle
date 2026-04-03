#include "render/Material.h"

#include "Logger.h"

#include <algorithm>

namespace OGLE {
    void Material::Bind(GLuint program) const
    {
        const GLint baseColorLocation = glGetUniformLocation(program, "uBaseColor");
        if (baseColorLocation >= 0) {
            glUniform3f(baseColorLocation, m_baseColor.x, m_baseColor.y, m_baseColor.z);
        }
        const GLint emissiveColorLocation = glGetUniformLocation(program, "uEmissiveColor");
        if (emissiveColorLocation >= 0) {
            glUniform3f(emissiveColorLocation, m_emissiveColor.x, m_emissiveColor.y, m_emissiveColor.z);
        }
        const GLint uvTilingLocation = glGetUniformLocation(program, "uUvTiling");
        if (uvTilingLocation >= 0) {
            glUniform2f(uvTilingLocation, m_uvTiling.x, m_uvTiling.y);
        }
        const GLint uvOffsetLocation = glGetUniformLocation(program, "uUvOffset");
        if (uvOffsetLocation >= 0) {
            glUniform2f(uvOffsetLocation, m_uvOffset.x, m_uvOffset.y);
        }
        const GLint roughnessLocation = glGetUniformLocation(program, "uRoughness");
        if (roughnessLocation >= 0) {
            glUniform1f(roughnessLocation, m_roughness);
        }
        const GLint metallicLocation = glGetUniformLocation(program, "uMetallic");
        if (metallicLocation >= 0) {
            glUniform1f(metallicLocation, m_metallic);
        }
        const GLint alphaCutoffLocation = glGetUniformLocation(program, "uAlphaCutoff");
        if (alphaCutoffLocation >= 0) {
            glUniform1f(alphaCutoffLocation, m_alphaCutoff);
        }

        const GLint hasTextureLocation = glGetUniformLocation(program, "uHasDiffuseTexture");
        const GLint textureSamplerLocation = glGetUniformLocation(program, "uDiffuseTexture");
        const GLint hasEmissiveTextureLocation = glGetUniformLocation(program, "uHasEmissiveTexture");
        const GLint emissiveTextureSamplerLocation = glGetUniformLocation(program, "uEmissiveTexture");

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

        glActiveTexture(GL_TEXTURE1);
        if (m_emissiveTexture && m_emissiveTexture->IsValid()) {
            glBindTexture(GL_TEXTURE_2D, m_emissiveTexture->GetTextureId());
            if (hasEmissiveTextureLocation >= 0) {
                glUniform1i(hasEmissiveTextureLocation, 1);
            }
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
            if (hasEmissiveTextureLocation >= 0) {
                glUniform1i(hasEmissiveTextureLocation, 0);
            }
        }

        if (emissiveTextureSamplerLocation >= 0) {
            glUniform1i(emissiveTextureSamplerLocation, 1);
        }

        glActiveTexture(GL_TEXTURE0);
    }

    void Material::SetBaseColor(const glm::vec3& color)
    {
        m_baseColor = color;
    }

    const glm::vec3& Material::GetBaseColor() const
    {
        return m_baseColor;
    }

    void Material::SetEmissiveColor(const glm::vec3& color)
    {
        m_emissiveColor = color;
    }

    const glm::vec3& Material::GetEmissiveColor() const
    {
        return m_emissiveColor;
    }

    void Material::SetUvTiling(const glm::vec2& tiling)
    {
        m_uvTiling = tiling;
    }

    const glm::vec2& Material::GetUvTiling() const
    {
        return m_uvTiling;
    }

    void Material::SetUvOffset(const glm::vec2& offset)
    {
        m_uvOffset = offset;
    }

    const glm::vec2& Material::GetUvOffset() const
    {
        return m_uvOffset;
    }

    void Material::SetRoughness(float roughness)
    {
        m_roughness = std::clamp(roughness, 0.0f, 1.0f);
    }

    float Material::GetRoughness() const
    {
        return m_roughness;
    }

    void Material::SetMetallic(float metallic)
    {
        m_metallic = std::clamp(metallic, 0.0f, 1.0f);
    }

    float Material::GetMetallic() const
    {
        return m_metallic;
    }

    void Material::SetAlphaCutoff(float alphaCutoff)
    {
        m_alphaCutoff = std::clamp(alphaCutoff, 0.0f, 1.0f);
    }

    float Material::GetAlphaCutoff() const
    {
        return m_alphaCutoff;
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

    bool Material::SetEmissiveTexturePath(const std::string& path)
    {
        if (path.empty()) {
            ClearEmissiveTexture();
            return true;
        }

        std::shared_ptr<Texture2D> texture = Texture2D::LoadShared(path);
        if (!texture || !texture->IsValid()) {
            LOG_WARN("Failed to load emissive texture: " + path);
            return false;
        }

        m_emissiveTexture = std::move(texture);
        m_emissiveTexturePath = m_emissiveTexture->GetPath();
        return true;
    }

    void Material::ClearEmissiveTexture()
    {
        m_emissiveTexture.reset();
        m_emissiveTexturePath.clear();
    }

    bool Material::HasEmissiveTexture() const
    {
        return m_emissiveTexture && m_emissiveTexture->IsValid();
    }

    const std::string& Material::GetEmissiveTexturePath() const
    {
        return m_emissiveTexturePath;
    }

    void Material::SetShaderProgram(const std::string& shaderProgramName)
    {
        m_shaderProgramName = shaderProgramName;
    }

    const std::string& Material::GetShaderProgram() const
    {
        return m_shaderProgramName;
    }
}
