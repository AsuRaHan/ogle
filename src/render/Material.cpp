#include "render/Material.h"
#include "opengl/ShaderManager.h"
#include "Logger.h"
#include "render/TextureManager.h"

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

        // Texture handling
        int textureUnit = 0;
        for (const auto& pair : m_textureSlots) {
            const std::string& slotName = pair.first;
            const std::shared_ptr<Texture2D>& texture = pair.second;

            if (texture && texture->IsValid()) {
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, texture->GetTextureId());
                m_shader->SetUniform("uTexture_" + slotName, textureUnit);
                m_shader->SetUniform("uHasTexture_" + slotName, 1);
                textureUnit++;
            } else {
                m_shader->SetUniform("uHasTexture_" + slotName, 0);
            }
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

    void Material::AddTexture(const std::string& slotName, const std::string& texturePath)
    {
        if (texturePath.empty()) {
            RemoveTexture(slotName);
            return;
        }

        std::shared_ptr<Texture2D> texture = TextureManager::Get().Load(texturePath);
        if (!texture || !texture->IsValid()) {
            LOG_WARN("Failed to load texture for slot '" + slotName + "': " + texturePath);
            // Even if it fails, we store the path so it can be fixed in the editor
            m_textureSlotPaths[slotName] = texturePath;
            m_textureSlots.erase(slotName);
            return;
        }

        m_textureSlots[slotName] = texture;
        m_textureSlotPaths[slotName] = texture->GetPath(); // Use resolved path
    }

    void Material::RemoveTexture(const std::string& slotName)
    {
        m_textureSlots.erase(slotName);
        m_textureSlotPaths.erase(slotName);
    }

    std::shared_ptr<Texture2D> Material::GetTexture(const std::string& slotName) const
    {
        auto it = m_textureSlots.find(slotName);
        if (it != m_textureSlots.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::map<std::string, std::string>& Material::GetTexturePaths() const
    {
        return m_textureSlotPaths;
    }

    bool Material::HasTexture(const std::string& slotName) const
    {
        auto it = m_textureSlots.find(slotName);
        return it != m_textureSlots.end() && it->second && it->second->IsValid();
    }

    nlohmann::json Material::ToJson() const
    {
        nlohmann::json j;
        j["baseColor"] = {m_baseColor.x, m_baseColor.y, m_baseColor.z};
        j["emissiveColor"] = {m_emissiveColor.x, m_emissiveColor.y, m_emissiveColor.z};
        j["uvTiling"] = {m_uvTiling.x, m_uvTiling.y};
        j["uvOffset"] = {m_uvOffset.x, m_uvOffset.y};
        j["roughness"] = m_roughness;
        j["metallic"] = m_metallic;
        j["alphaCutoff"] = m_alphaCutoff;
        j["shaderProgram"] = m_shaderProgramName;
        j["textureSlots"] = m_textureSlotPaths;
        return j;
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
        
        if (j.contains("textureSlots")) {
            m_textureSlotPaths.clear();
            m_textureSlots.clear();
            const auto& texturesJson = j.at("textureSlots");
            for (auto it = texturesJson.begin(); it != texturesJson.end(); ++it) {
                AddTexture(it.key(), it.value().get<std::string>());
            }
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
