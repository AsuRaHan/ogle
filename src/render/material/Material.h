// src/render/material/Material.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include "log/Logger.h"
#include "render/shader/ShaderProgram.h"

// Предварительное объявление
namespace ogle {
    class ShaderProgram;
    class Texture;
}

namespace ogle {

    enum class MaterialType {
        Basic,
        PBR,
        Unlit,
        Custom
    };

// Состояние рендеринга
struct RenderState {
    bool depthTest = true;
    bool depthWrite = true;
    enum DepthFunc { Less, LessEqual, Always } depthFunc = Less;
    
    bool cullFace = true;
    enum CullMode { Back, Front, None } cullMode = Back;
    
    bool blend = false;
    enum BlendFactor {
        Zero, One,
        SrcColor, OneMinusSrcColor,
        DstColor, OneMinusDstColor,
        SrcAlpha, OneMinusSrcAlpha,
        DstAlpha, OneMinusDstAlpha
    } blendSrc = SrcAlpha, blendDst = OneMinusSrcAlpha;
    
    bool wireframe = false;
    float lineWidth = 1.0f;
    
    void Apply() const;
};

// Базовый класс материала
class Material {
public:
    virtual ~Material() = default;
    
    // Виртуальные методы
    virtual void Apply(ShaderProgram* shader) = 0;
    virtual void BindTextures() = 0;
    virtual std::unique_ptr<Material> Clone() const = 0;
    
    // Общие свойства
    std::string GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    
    ShaderProgram* GetShader() const { return m_shader; }
    void SetShader(ShaderProgram* shader) { m_shader = shader; }
    
    const RenderState& GetRenderState() const { return m_renderState; }
    RenderState& GetRenderState() { return m_renderState; }
    void SetRenderState(const RenderState& state) { m_renderState = state; }
    
    // Параметры материала (универсальные)
    void SetFloat(const std::string& name, float value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetTexture(const std::string& name, Texture* texture);
    
    float GetFloat(const std::string& name, float defaultValue = 0.0f) const;
    glm::vec3 GetVec3(const std::string& name, const glm::vec3& defaultValue = glm::vec3(0)) const;
    glm::vec4 GetVec4(const std::string& name, const glm::vec4& defaultValue = glm::vec4(0)) const;
    Texture* GetTexture(const std::string& name) const;
    
    // Применение всех параметров к шейдеру
    void ApplyParameters(ShaderProgram* shader) const;
    
protected:
    std::string m_name = "UnnamedMaterial";
    ShaderProgram* m_shader = nullptr;
    RenderState m_renderState;
    
    // Динамические параметры
    std::unordered_map<std::string, float> m_floatParams;
    std::unordered_map<std::string, glm::vec3> m_vec3Params;
    std::unordered_map<std::string, glm::vec4> m_vec4Params;
    std::unordered_map<std::string, Texture*> m_textureParams;
};

// Конкретные типы материалов
class BasicMaterial : public Material {
public:
    BasicMaterial();
    ~BasicMaterial() override = default;
    
    void Apply(ShaderProgram* shader) override;
    void BindTextures() override;
    std::unique_ptr<Material> Clone() const override;
    
    // Специфичные свойства
    void SetColor(const glm::vec4& color);
    glm::vec4 GetColor() const;
    
    void SetTexture(Texture* texture);
    Texture* GetTexture() const;
    
private:
    glm::vec4 m_color = {1.0f, 1.0f, 1.0f, 1.0f};
    Texture* m_texture = nullptr;
};

class PBRMaterial : public Material {
public:
    PBRMaterial();
    ~PBRMaterial() override = default;
    
    void Apply(ShaderProgram* shader) override;
    void BindTextures() override;
    std::unique_ptr<Material> Clone() const override;
    
    // PBR свойства
    void SetAlbedo(const glm::vec4& albedo);
    void SetMetallic(float metallic);
    void SetRoughness(float roughness);
    void SetAO(float ao);
    void SetEmission(const glm::vec3& emission);
    
    glm::vec4 GetAlbedo() const;
    float GetMetallic() const;
    float GetRoughness() const;
    float GetAO() const;
    glm::vec3 GetEmission() const;
    
    // Текстуры
    void SetAlbedoMap(Texture* texture);
    void SetNormalMap(Texture* texture);
    void SetMetallicRoughnessMap(Texture* texture);
    void SetAOMap(Texture* texture);
    void SetEmissionMap(Texture* texture);
    
    Texture* GetAlbedoMap() const;
    Texture* GetNormalMap() const;
    Texture* GetMetallicRoughnessMap() const;
    Texture* GetAOMap() const;
    Texture* GetEmissionMap() const;
    
private:
    // Значения по умолчанию
    glm::vec4 m_albedo = {0.8f, 0.8f, 0.8f, 1.0f};
    float m_metallic = 0.0f;
    float m_roughness = 0.5f;
    float m_ao = 1.0f;
    glm::vec3 m_emission = {0.0f, 0.0f, 0.0f};
    
    // Текстуры
    Texture* m_albedoMap = nullptr;
    Texture* m_normalMap = nullptr;
    Texture* m_metallicRoughnessMap = nullptr;
    Texture* m_aoMap = nullptr;
    Texture* m_emissionMap = nullptr;
};

// Material Instance (для переопределения параметров)
class MaterialInstance : public Material {
public:
    MaterialInstance(Material* baseMaterial);
    ~MaterialInstance() override = default;
    
    void Apply(ShaderProgram* shader) override;
    void BindTextures() override;
    std::unique_ptr<Material> Clone() const override;
    
    // Переопределение параметров
    void SetBaseMaterial(Material* material);
    Material* GetBaseMaterial() const;
    
    // Проверка, переопределен ли параметр
    bool IsFloatOverridden(const std::string& name) const;
    bool IsVec3Overridden(const std::string& name) const;
    bool IsVec4Overridden(const std::string& name) const;
    bool IsTextureOverridden(const std::string& name) const;
    
    // Сброс переопределений
    void ClearFloatOverride(const std::string& name);
    void ClearVec3Override(const std::string& name);
    void ClearVec4Override(const std::string& name);
    void ClearTextureOverride(const std::string& name);
    void ClearAllOverrides();
    
private:
    Material* m_baseMaterial = nullptr;
    
    // Только переопределенные значения
    std::unordered_map<std::string, float> m_floatOverrides;
    std::unordered_map<std::string, glm::vec3> m_vec3Overrides;
    std::unordered_map<std::string, glm::vec4> m_vec4Overrides;
    std::unordered_map<std::string, Texture*> m_textureOverrides;
};

} // namespace ogle