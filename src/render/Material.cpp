#include "render/Material.h"
#include "opengl/ShaderManager.h"
#include "Logger.h"

#include <algorithm>

namespace OGLE {
    void Material::Bind() const
    {
        // Use the stored shader object to set uniforms via its cache
        if (!m_shader) return;
        m_shader->Bind();
        // Basic material properties
        m_shader->SetUniform("uBaseColor", m_baseColor);
        m_shader->SetUniform("uEmissiveColor", m_emissiveColor);
        m_shader->SetUniform("uUvTiling", m_uvTiling);
        m_shader->SetUniform("uUvOffset", m_uvOffset);
        m_shader->SetUniform("uRoughness", m_roughness);
        m_shader->SetUniform("uMetallic", m_metallic);
        m_shader->SetUniform("uAlphaCutoff", m_alphaCutoff);

        // Texture handling – set flags and bind textures
        int hasDiffuse = (m_diffuseTexture && m_diffuseTexture->IsValid()) ? 1 : 0;
        int hasEmissive = (m_emissiveTexture && m_emissiveTexture->IsValid()) ? 1 : 0;
        m_shader->SetUniform("uHasDiffuseTexture", hasDiffuse);
        m_shader->SetUniform("uHasEmissiveTexture", hasEmissive);

        // Bind textures to units 0 and 1
        glActiveTexture(GL_TEXTURE0);
        if (m_diffuseTexture && m_diffuseTexture->IsValid()) {
            glBindTexture(GL_TEXTURE_2D, m_diffuseTexture->GetTextureId());
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE1);
        if (m_emissiveTexture && m_emissiveTexture->IsValid()) {
            glBindTexture(GL_TEXTURE_2D, m_emissiveTexture->GetTextureId());
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // Sampler uniforms
        m_shader->SetUniform("uDiffuseTexture", 0);
        m_shader->SetUniform("uEmissiveTexture", 1);
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

    nlohmann::json Material::ToJson() const
    {
        return nlohmann::json{
            {"baseColor", {m_baseColor.x, m_baseColor.y, m_baseColor.z}},
            {"emissiveColor", {m_emissiveColor.x, m_emissiveColor.y, m_emissiveColor.z}},
            {"uvTiling", {m_uvTiling.x, m_uvTiling.y}},
            {"uvOffset", {m_uvOffset.x, m_uvOffset.y}},
            {"roughness", m_roughness},
            {"metallic", m_metallic},
            {"alphaCutoff", m_alphaCutoff},
            {"diffuseTexturePath", m_diffuseTexturePath},
            {"emissiveTexturePath", m_emissiveTexturePath},
            {"shaderProgram", m_shaderProgramName}
        };
    }

    bool Material::FromJson(const nlohmann::json& j)
    {
        if (j.contains("baseColor")) {
            auto c = j.at("baseColor");
            SetBaseColor(glm::vec3(c[0], c[1], c[2]));
        }
        if (j.contains("emissiveColor")) {
            auto c = j.at("emissiveColor");
            SetEmissiveColor(glm::vec3(c[0], c[1], c[2]));
        }
        if (j.contains("uvTiling")) {
            auto v = j.at("uvTiling");
            SetUvTiling(glm::vec2(v[0], v[1]));
        }
        if (j.contains("uvOffset")) {
            auto v = j.at("uvOffset");
            SetUvOffset(glm::vec2(v[0], v[1]));
        }
        if (j.contains("roughness")) {
            SetRoughness(j.at("roughness").get<float>());
        }
        if (j.contains("metallic")) {
            SetMetallic(j.at("metallic").get<float>());
        }
        if (j.contains("alphaCutoff")) {
            SetAlphaCutoff(j.at("alphaCutoff").get<float>());
        }
        if (j.contains("diffuseTexturePath")) {
            SetDiffuseTexturePath(j.at("diffuseTexturePath").get<std::string>());
        }
        if (j.contains("emissiveTexturePath")) {
            SetEmissiveTexturePath(j.at("emissiveTexturePath").get<std::string>());
        }
        if (j.contains("shaderProgram")) {
            SetShaderProgram(j.at("shaderProgram").get<std::string>());
        }

        return true;
    }

    void Material::SetShaderProgram(const std::string& shaderProgramName)
    {
        m_shaderProgramName = shaderProgramName;
        if (ShaderManager::GetGlobalInstance()) {
            m_shader = ShaderManager::GetGlobalInstance()->GetShaderProgram(shaderProgramName);
        }
    }

    const std::string& Material::GetShaderProgram() const
    {
        return m_shaderProgramName;
    }
}
