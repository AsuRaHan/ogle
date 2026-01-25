// src/render/material/Material.cpp (исправленный)
#include "Material.h"
#include "render/ShaderController.h"

namespace ogle {

// ==================== Material ====================

void Material::SetFloat(const std::string& name, float value) {
    m_floatParams[name] = value;
}

void Material::SetVec3(const std::string& name, const glm::vec3& value) {
    m_vec3Params[name] = value;
}

void Material::SetVec4(const std::string& name, const glm::vec4& value) {
    m_vec4Params[name] = value;
}

void Material::SetTexture(const std::string& name, Texture* texture) {
    m_textureParams[name] = texture;
}

float Material::GetFloat(const std::string& name, float defaultValue) const {
    auto it = m_floatParams.find(name);
    return it != m_floatParams.end() ? it->second : defaultValue;
}

glm::vec3 Material::GetVec3(const std::string& name, const glm::vec3& defaultValue) const {
    auto it = m_vec3Params.find(name);
    return it != m_vec3Params.end() ? it->second : defaultValue;
}

glm::vec4 Material::GetVec4(const std::string& name, const glm::vec4& defaultValue) const {
    auto it = m_vec4Params.find(name);
    return it != m_vec4Params.end() ? it->second : defaultValue;
}

Texture* Material::GetTexture(const std::string& name) const {
    auto it = m_textureParams.find(name);
    return it != m_textureParams.end() ? it->second : nullptr;
}

void Material::ApplyParameters(ShaderProgram* shader) const {
    if (!shader) return;
    
    // Применяем float параметры
    for (const auto& param : m_floatParams) {
        shader->SetFloat(param.first, param.second);
    }
    
    // Применяем vec3 параметры
    for (const auto& param : m_vec3Params) {
        shader->SetVec3(param.first, param.second);
    }
    
    // Применяем vec4 параметры
    for (const auto& param : m_vec4Params) {
        shader->SetVec4(param.first, param.second);
    }
    
    // Применяем bool параметры (через int)
    for (const auto& param : m_textureParams) {
        // Если текстура существует - true, иначе false
        shader->SetInt(param.first + "_enabled", param.second ? 1 : 0);
    }
}

// ==================== BasicMaterial ====================

BasicMaterial::BasicMaterial() {
    m_name = "BasicMaterial";
}

void BasicMaterial::Apply(ShaderProgram* shader) {
    if (!shader) return;
    
    // Применяем общие параметры
    ApplyParameters(shader);
    
    // Применяем специфичные параметры
    shader->SetVec4("uColor", m_color);
    
    // Используем SetInt вместо SetBool (если SetBool нет)
    shader->SetInt("uUseTexture", m_texture != nullptr ? 1 : 0);
}

void BasicMaterial::BindTextures() {
    if (m_texture) {
        // TODO: Привязать текстуру
        // m_texture->Bind(0);
        // Пока просто логируем
        // Logger::Debug("Texture would be bound here");
    }
}

std::unique_ptr<Material> BasicMaterial::Clone() const {
    auto clone = std::make_unique<BasicMaterial>();
    clone->m_name = m_name + "_Copy";
    clone->m_shader = m_shader;
    clone->m_renderState = m_renderState;
    clone->m_color = m_color;
    clone->m_texture = m_texture;
    
    // Копируем параметры
    clone->m_floatParams = m_floatParams;
    clone->m_vec3Params = m_vec3Params;
    clone->m_vec4Params = m_vec4Params;
    clone->m_textureParams = m_textureParams;
    
    return clone;
}

void BasicMaterial::SetColor(const glm::vec4& color) {
    m_color = color;
    SetVec4("uColor", color); // Дублируем в параметры
}

glm::vec4 BasicMaterial::GetColor() const {
    return m_color;
}

void BasicMaterial::SetTexture(Texture* texture) {
    m_texture = texture;
    // ИСПРАВЛЕНО: используем правильный метод SetTexture
    // с двумя параметрами из базового класса
    Material::SetTexture("uTexture", texture);
}

Texture* BasicMaterial::GetTexture() const {
    return m_texture;
}

// ==================== PBRMaterial ====================

PBRMaterial::PBRMaterial() {
    m_name = "PBRMaterial";
}

void PBRMaterial::Apply(ShaderProgram* shader) {
    if (!shader) return;
    
    // Применяем общие параметры
    ApplyParameters(shader);
    
    // Применяем PBR параметры
    shader->SetVec4("uAlbedo", m_albedo);
    shader->SetFloat("uMetallic", m_metallic);
    shader->SetFloat("uRoughness", m_roughness);
    shader->SetFloat("uAO", m_ao);
    shader->SetVec3("uEmission", m_emission);
    
    // Флаги наличия текстур
    shader->SetInt("uHasAlbedoMap", m_albedoMap ? 1 : 0);
    shader->SetInt("uHasNormalMap", m_normalMap ? 1 : 0);
    shader->SetInt("uHasMetallicRoughnessMap", m_metallicRoughnessMap ? 1 : 0);
    shader->SetInt("uHasAOMap", m_aoMap ? 1 : 0);
    shader->SetInt("uHasEmissionMap", m_emissionMap ? 1 : 0);
}

void PBRMaterial::BindTextures() {
    // TODO: Привязать PBR текстуры
    // Пока просто логируем
    /*
    if (m_albedoMap) Logger::Debug("Albedo map would be bound");
    if (m_normalMap) Logger::Debug("Normal map would be bound");
    if (m_metallicRoughnessMap) Logger::Debug("MetallicRoughness map would be bound");
    if (m_aoMap) Logger::Debug("AO map would be bound");
    if (m_emissionMap) Logger::Debug("Emission map would be bound");
    */
}

std::unique_ptr<Material> PBRMaterial::Clone() const {
    auto clone = std::make_unique<PBRMaterial>();
    
    // Копируем базовые свойства
    clone->m_name = m_name + "_Copy";
    clone->m_shader = m_shader;
    clone->m_renderState = m_renderState;
    
    // Копируем PBR свойства
    clone->m_albedo = m_albedo;
    clone->m_metallic = m_metallic;
    clone->m_roughness = m_roughness;
    clone->m_ao = m_ao;
    clone->m_emission = m_emission;
    
    // Копируем текстуры (только указатели)
    clone->m_albedoMap = m_albedoMap;
    clone->m_normalMap = m_normalMap;
    clone->m_metallicRoughnessMap = m_metallicRoughnessMap;
    clone->m_aoMap = m_aoMap;
    clone->m_emissionMap = m_emissionMap;
    
    // Копируем параметры
    clone->m_floatParams = m_floatParams;
    clone->m_vec3Params = m_vec3Params;
    clone->m_vec4Params = m_vec4Params;
    clone->m_textureParams = m_textureParams;
    
    return clone;
}

// Геттеры/сеттеры PBRMaterial
void PBRMaterial::SetAlbedo(const glm::vec4& albedo) { 
    m_albedo = albedo; 
    SetVec4("uAlbedo", albedo);
}

void PBRMaterial::SetMetallic(float metallic) { 
    m_metallic = metallic; 
    SetFloat("uMetallic", metallic);
}

void PBRMaterial::SetRoughness(float roughness) { 
    m_roughness = roughness; 
    SetFloat("uRoughness", roughness);
}

void PBRMaterial::SetAO(float ao) { 
    m_ao = ao; 
    SetFloat("uAO", ao);
}

void PBRMaterial::SetEmission(const glm::vec3& emission) { 
    m_emission = emission; 
    SetVec3("uEmission", emission);
}

glm::vec4 PBRMaterial::GetAlbedo() const { return m_albedo; }
float PBRMaterial::GetMetallic() const { return m_metallic; }
float PBRMaterial::GetRoughness() const { return m_roughness; }
float PBRMaterial::GetAO() const { return m_ao; }
glm::vec3 PBRMaterial::GetEmission() const { return m_emission; }

// Методы для текстур
void PBRMaterial::SetAlbedoMap(Texture* texture) { 
    m_albedoMap = texture; 
    SetTexture("uAlbedoMap", texture);
}

void PBRMaterial::SetNormalMap(Texture* texture) { 
    m_normalMap = texture; 
    SetTexture("uNormalMap", texture);
}

void PBRMaterial::SetMetallicRoughnessMap(Texture* texture) { 
    m_metallicRoughnessMap = texture; 
    SetTexture("uMetallicRoughnessMap", texture);
}

void PBRMaterial::SetAOMap(Texture* texture) { 
    m_aoMap = texture; 
    SetTexture("uAOMap", texture);
}

void PBRMaterial::SetEmissionMap(Texture* texture) { 
    m_emissionMap = texture; 
    SetTexture("uEmissionMap", texture);
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
    
    // Затем переопределенные параметры (перезаписывают базовые)
    for (const auto& param : m_floatOverrides) {
        shader->SetFloat(param.first, param.second);
    }
    for (const auto& param : m_vec3Overrides) {
        shader->SetVec3(param.first, param.second);
    }
    for (const auto& param : m_vec4Overrides) {
        shader->SetVec4(param.first, param.second);
    }
}

void MaterialInstance::BindTextures() {
    if (!m_baseMaterial) return;
    
    // Сначала привязываем текстуры базового материала
    m_baseMaterial->BindTextures();
    
    // TODO: Затем переопределенные текстуры
    // Пока что просто используем базовые
}

std::unique_ptr<Material> MaterialInstance::Clone() const {
    auto clone = std::make_unique<MaterialInstance>(m_baseMaterial);
    
    // Копируем базовые свойства
    clone->m_name = m_name + "_Copy";
    clone->m_shader = m_shader;
    clone->m_renderState = m_renderState;
    
    // Копируем переопределения
    clone->m_floatOverrides = m_floatOverrides;
    clone->m_vec3Overrides = m_vec3Overrides;
    clone->m_vec4Overrides = m_vec4Overrides;
    clone->m_textureOverrides = m_textureOverrides;
    
    // Копируем параметры (наследуются от Material)
    clone->m_floatParams = m_floatParams;
    clone->m_vec3Params = m_vec3Params;
    clone->m_vec4Params = m_vec4Params;
    clone->m_textureParams = m_textureParams;
    
    return clone;
}

void MaterialInstance::SetBaseMaterial(Material* material) {
    m_baseMaterial = material;
}

Material* MaterialInstance::GetBaseMaterial() const {
    return m_baseMaterial;
}

bool MaterialInstance::IsFloatOverridden(const std::string& name) const {
    return m_floatOverrides.find(name) != m_floatOverrides.end();
}

bool MaterialInstance::IsVec3Overridden(const std::string& name) const {
    return m_vec3Overrides.find(name) != m_vec3Overrides.end();
}

bool MaterialInstance::IsVec4Overridden(const std::string& name) const {
    return m_vec4Overrides.find(name) != m_vec4Overrides.end();
}

bool MaterialInstance::IsTextureOverridden(const std::string& name) const {
    return m_textureOverrides.find(name) != m_textureOverrides.end();
}

void MaterialInstance::ClearFloatOverride(const std::string& name) {
    m_floatOverrides.erase(name);
}

void MaterialInstance::ClearVec3Override(const std::string& name) {
    m_vec3Overrides.erase(name);
}

void MaterialInstance::ClearVec4Override(const std::string& name) {
    m_vec4Overrides.erase(name);
}

void MaterialInstance::ClearTextureOverride(const std::string& name) {
    m_textureOverrides.erase(name);
}

void MaterialInstance::ClearAllOverrides() {
    m_floatOverrides.clear();
    m_vec3Overrides.clear();
    m_vec4Overrides.clear();
    m_textureOverrides.clear();
}

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