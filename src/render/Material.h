#pragma once

#include "Texture2D.h"

#include <glm/vec3.hpp>

#include <memory>
#include <string>

namespace OGLE {
    class Material {
    public:
        void Bind(GLuint program) const;

        void SetBaseColor(const glm::vec3& color);
        const glm::vec3& GetBaseColor() const;

        bool SetDiffuseTexturePath(const std::string& path);
        void ClearDiffuseTexture();
        bool HasDiffuseTexture() const;
        const std::string& GetDiffuseTexturePath() const;

    private:
        glm::vec3 m_baseColor{ 1.0f, 1.0f, 1.0f };
        std::string m_diffuseTexturePath;
        std::shared_ptr<Texture2D> m_diffuseTexture;
    };
}
