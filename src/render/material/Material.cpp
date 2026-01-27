// src/render/material/Material.cpp
#include "Material.h"
#include "render/shader/ShaderProgram.h"
#include "render/texture/Texture.h"

namespace ogle {

    // ==================== Material::ApplyParameters ====================

    void Material::ApplyParameters(ShaderProgram* shader) const {
        if (!shader) return;

        // Применяем float параметры
        for (const auto& param : m_floatParams) {
            shader->SetFloat(param.first, param.second);
        }

        // Применяем int параметры
        for (const auto& param : m_intParams) {
            shader->SetInt(param.first, param.second);
        }

        // Применяем vec3 параметры
        for (const auto& param : m_vec3Params) {
            shader->SetVec3(param.first, param.second);
        }

        // Применяем vec4 параметры
        for (const auto& param : m_vec4Params) {
            shader->SetVec4(param.first, param.second);
        }
    }

    // ==================== BasicMaterial ====================

    BasicMaterial::BasicMaterial() {
        m_name = "BasicMaterial";
        // Значения по умолчанию
        SetVec4("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        SetInt("uUseTexture", 0);
        SetInt("uUseLighting", 1);
    }

    void BasicMaterial::Apply(ShaderProgram* shader) {
        if (!shader) return;

        // Применяем все параметры к шейдеру
        ApplyParameters(shader);

        // Привязываем текстуры
        BindTextures();
    }

    void BasicMaterial::BindTextures() {
        // Привязываем все текстуры этого материала
        GLuint unit = 0;
        for (const auto& pair : m_textureParams) {
            if (pair.second) {
                auto it = m_textureUnits.find(pair.first);
                GLuint texUnit = (it != m_textureUnits.end()) ? it->second : unit;
                pair.second->Bind(texUnit);
                unit++;
            }
        }
    }

    std::unique_ptr<Material> BasicMaterial::Clone() const {
        auto clone = std::make_unique<BasicMaterial>();

        // Копируем общие свойства
        clone->m_name = m_name + "_Copy";
        clone->m_shader = m_shader;
        clone->m_renderState = m_renderState;

        // Копируем все параметры
        clone->m_floatParams = m_floatParams;
        clone->m_vec3Params = m_vec3Params;
        clone->m_vec4Params = m_vec4Params;
        clone->m_intParams = m_intParams;
        clone->m_textureParams = m_textureParams;
        clone->m_textureUnits = m_textureUnits;

        return clone;
    }

    // ==================== PBRMaterial ====================

    PBRMaterial::PBRMaterial() {
        m_name = "PBRMaterial";
        // Значения по умолчанию для PBR
        SetVec4("uAlbedo", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        SetFloat("uMetallic", 0.0f);
        SetFloat("uRoughness", 0.5f);
        SetFloat("uAO", 1.0f);
        SetVec3("uEmission", glm::vec3(0.0f, 0.0f, 0.0f));
    }

    void PBRMaterial::Apply(ShaderProgram* shader) {
        if (!shader) return;

        // Применяем все параметры к шейдеру
        ApplyParameters(shader);

        // Привязываем текстуры
        BindTextures();
    }

    void PBRMaterial::BindTextures() {
        // Привязываем PBR текстуры в определенном порядке
        GLuint unit = 0;

        // Albedo map
        if (m_albedoMap) {
            m_albedoMap->Bind(unit);
            SetInt("uAlbedoMap", unit);
            SetInt("uHasAlbedoMap", 1);
            unit++;
        }
        else {
            SetInt("uHasAlbedoMap", 0);
        }

        // Normal map
        if (m_normalMap) {
            m_normalMap->Bind(unit);
            SetInt("uNormalMap", unit);
            SetInt("uHasNormalMap", 1);
            unit++;
        }
        else {
            SetInt("uHasNormalMap", 0);
        }

        // MetallicRoughness map
        if (m_metallicRoughnessMap) {
            m_metallicRoughnessMap->Bind(unit);
            SetInt("uMetallicRoughnessMap", unit);
            SetInt("uHasMetallicRoughnessMap", 1);
            unit++;
        }
        else {
            SetInt("uHasMetallicRoughnessMap", 0);
        }

        // AO map
        if (m_aoMap) {
            m_aoMap->Bind(unit);
            SetInt("uAOMap", unit);
            SetInt("uHasAOMap", 1);
            unit++;
        }
        else {
            SetInt("uHasAOMap", 0);
        }

        // Emission map
        if (m_emissionMap) {
            m_emissionMap->Bind(unit);
            SetInt("uEmissionMap", unit);
            SetInt("uHasEmissionMap", 1);
            unit++;
        }
        else {
            SetInt("uHasEmissionMap", 0);
        }
    }

    std::unique_ptr<Material> PBRMaterial::Clone() const {
        auto clone = std::make_unique<PBRMaterial>();

        // Копируем общие свойства
        clone->m_name = m_name + "_Copy";
        clone->m_shader = m_shader;
        clone->m_renderState = m_renderState;

        // Копируем все параметры
        clone->m_floatParams = m_floatParams;
        clone->m_vec3Params = m_vec3Params;
        clone->m_vec4Params = m_vec4Params;
        clone->m_intParams = m_intParams;
        clone->m_textureParams = m_textureParams;
        clone->m_textureUnits = m_textureUnits;

        // Копируем PBR текстуры
        clone->m_albedoMap = m_albedoMap;
        clone->m_normalMap = m_normalMap;
        clone->m_metallicRoughnessMap = m_metallicRoughnessMap;
        clone->m_aoMap = m_aoMap;
        clone->m_emissionMap = m_emissionMap;

        return clone;
    }

    // Методы для текстур PBRMaterial
    void PBRMaterial::SetAlbedoMap(Texture* texture) {
        m_albedoMap = texture;
        Material::SetTexture("uAlbedoMap", texture);
    }

    void PBRMaterial::SetNormalMap(Texture* texture) {
        m_normalMap = texture;
        Material::SetTexture("uNormalMap", texture);
    }

    void PBRMaterial::SetMetallicRoughnessMap(Texture* texture) {
        m_metallicRoughnessMap = texture;
        Material::SetTexture("uMetallicRoughnessMap", texture);
    }

    void PBRMaterial::SetAOMap(Texture* texture) {
        m_aoMap = texture;
        Material::SetTexture("uAOMap", texture);
    }

    void PBRMaterial::SetEmissionMap(Texture* texture) {
        m_emissionMap = texture;
        Material::SetTexture("uEmissionMap", texture);
    }

    Texture* PBRMaterial::GetAlbedoMap() const { return m_albedoMap; }
    Texture* PBRMaterial::GetNormalMap() const { return m_normalMap; }
    Texture* PBRMaterial::GetMetallicRoughnessMap() const { return m_metallicRoughnessMap; }
    Texture* PBRMaterial::GetAOMap() const { return m_aoMap; }
    Texture* PBRMaterial::GetEmissionMap() const { return m_emissionMap; }

    // ==================== MaterialInstance ====================

    MaterialInstance::MaterialInstance(Material* baseMaterial)
        : m_baseMaterial(baseMaterial) {
        if (baseMaterial) {
            m_name = baseMaterial->GetName() + "_Instance";
            m_shader = baseMaterial->GetShader();
            m_renderState = baseMaterial->GetRenderState();
        }
    }

    void MaterialInstance::Apply(ShaderProgram* shader) {
        if (!m_baseMaterial || !shader) return;

        // Сначала применяем базовый материал
        m_baseMaterial->Apply(shader);

        // Затем применяем наши переопределения поверх
        ApplyParameters(shader);
    }

    void MaterialInstance::BindTextures() {
        if (!m_baseMaterial) return;

        // Сначала привязываем текстуры базового материала
        m_baseMaterial->BindTextures();

        // Затем наши текстуры (переопределенные)
        for (const auto& pair : m_textureParams) {
            if (pair.second) {
                auto it = m_textureUnits.find(pair.first);
                GLuint unit = (it != m_textureUnits.end()) ? it->second : 0;
                pair.second->Bind(unit);
            }
        }
    }

    std::unique_ptr<Material> MaterialInstance::Clone() const {
        auto clone = std::make_unique<MaterialInstance>(m_baseMaterial);

        // Копируем свойства
        clone->m_name = m_name + "_Copy";
        clone->m_shader = m_shader;
        clone->m_renderState = m_renderState;

        // Копируем все параметры (они уже являются переопределениями)
        clone->m_floatParams = m_floatParams;
        clone->m_vec3Params = m_vec3Params;
        clone->m_vec4Params = m_vec4Params;
        clone->m_intParams = m_intParams;
        clone->m_textureParams = m_textureParams;
        clone->m_textureUnits = m_textureUnits;

        return clone;
    }

    void MaterialInstance::OverrideFloat(const std::string& name, float value) {
        SetFloat(name, value);
    }

    void MaterialInstance::OverrideVec3(const std::string& name, const glm::vec3& value) {
        SetVec3(name, value);
    }

    void MaterialInstance::OverrideVec4(const std::string& name, const glm::vec4& value) {
        SetVec4(name, value);
    }

    void MaterialInstance::OverrideTexture(const std::string& name, Texture* texture) {
        SetTexture(name, texture);
    }

    Material* MaterialInstance::GetBaseMaterial() const {
        return m_baseMaterial;
    }

    void MaterialInstance::SetBaseMaterial(Material* material) {
        m_baseMaterial = material;
    }

    // ==================== RenderState ====================

    void RenderState::Apply() const {
        // Depth test
        if (depthTest) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(depthFunc == Less ? GL_LESS :
                depthFunc == LessEqual ? GL_LEQUAL : GL_ALWAYS);
            glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }

        // Face culling
        if (cullFace) {
            glEnable(GL_CULL_FACE);
            glCullFace(cullMode == Back ? GL_BACK :
                cullMode == Front ? GL_FRONT : GL_NONE);
        }
        else {
            glDisable(GL_CULL_FACE);
        }

        // Blending
        if (blend) {
            glEnable(GL_BLEND);
            glBlendFunc(
                blendSrc == Zero ? GL_ZERO :
                blendSrc == One ? GL_ONE :
                blendSrc == SrcColor ? GL_SRC_COLOR :
                blendSrc == OneMinusSrcColor ? GL_ONE_MINUS_SRC_COLOR :
                blendSrc == DstColor ? GL_DST_COLOR :
                blendSrc == OneMinusDstColor ? GL_ONE_MINUS_DST_COLOR :
                blendSrc == SrcAlpha ? GL_SRC_ALPHA :
                blendSrc == OneMinusSrcAlpha ? GL_ONE_MINUS_SRC_ALPHA :
                blendSrc == DstAlpha ? GL_DST_ALPHA : GL_ONE_MINUS_DST_ALPHA,

                blendDst == Zero ? GL_ZERO :
                blendDst == One ? GL_ONE :
                blendDst == SrcColor ? GL_SRC_COLOR :
                blendDst == OneMinusSrcColor ? GL_ONE_MINUS_SRC_COLOR :
                blendDst == DstColor ? GL_DST_COLOR :
                blendDst == OneMinusDstColor ? GL_ONE_MINUS_DST_COLOR :
                blendDst == SrcAlpha ? GL_SRC_ALPHA :
                blendDst == OneMinusSrcAlpha ? GL_ONE_MINUS_SRC_ALPHA :
                blendDst == DstAlpha ? GL_DST_ALPHA : GL_ONE_MINUS_DST_ALPHA
            );
        }
        else {
            glDisable(GL_BLEND);
        }

        // Wireframe
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(lineWidth);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

} // namespace ogle