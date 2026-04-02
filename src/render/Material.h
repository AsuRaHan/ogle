#pragma once

#include "Texture2D.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <string>

namespace OGLE {
    class Material {
    public:
        void Bind(GLuint program) const;

        void SetBaseColor(const glm::vec3& color);
        const glm::vec3& GetBaseColor() const;
        void SetEmissiveColor(const glm::vec3& color);
        const glm::vec3& GetEmissiveColor() const;
        void SetUvTiling(const glm::vec2& tiling);
        const glm::vec2& GetUvTiling() const;
        void SetUvOffset(const glm::vec2& offset);
        const glm::vec2& GetUvOffset() const;
        void SetRoughness(float roughness);
        float GetRoughness() const;
        void SetMetallic(float metallic);
        float GetMetallic() const;
        void SetAlphaCutoff(float alphaCutoff);
        float GetAlphaCutoff() const;

        bool SetDiffuseTexturePath(const std::string& path);
        void ClearDiffuseTexture();
        bool HasDiffuseTexture() const;
        const std::string& GetDiffuseTexturePath() const;
        bool SetEmissiveTexturePath(const std::string& path);
        void ClearEmissiveTexture();
        bool HasEmissiveTexture() const;
        const std::string& GetEmissiveTexturePath() const;

    private:
        // Editable surface parameters live here so the editor, serializer, and renderer share one source of truth.
        glm::vec3 m_baseColor{ 1.0f, 1.0f, 1.0f };
        glm::vec3 m_emissiveColor{ 0.0f, 0.0f, 0.0f };
        glm::vec2 m_uvTiling{ 1.0f, 1.0f };
        glm::vec2 m_uvOffset{ 0.0f, 0.0f };
        float m_roughness = 0.7f;
        float m_metallic = 0.0f;
        float m_alphaCutoff = 0.0f;
        std::string m_diffuseTexturePath;
        std::shared_ptr<Texture2D> m_diffuseTexture;
        std::string m_emissiveTexturePath;
        std::shared_ptr<Texture2D> m_emissiveTexture;
    };
}
