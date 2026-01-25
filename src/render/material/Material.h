// src/render/material/Material.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <glad/gl.h>   // от GLAD 2
#include "log/Logger.h"
#include "render/ShaderController.h"
#include "render/shader/ShaderProgram.h"
#include "render/texture/Texture.h"

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
    void SetInt(const std::string& name, int value);
    void SetTexture(const std::string& name, Texture* texture);
    
    float GetFloat(const std::string& name, float defaultValue = 0.0f) const;
    glm::vec3 GetVec3(const std::string& name, const glm::vec3& defaultValue = glm::vec3(0)) const;
    glm::vec4 GetVec4(const std::string& name, const glm::vec4& defaultValue = glm::vec4(0)) const;
    int GetInt(const std::string& name, int defaultValue = 0) const;
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
    std::unordered_map<std::string, int> m_intParams;
    std::unordered_map<std::string, Texture*> m_textureParams;
    
    // Какие texture units использовать для каких текстур
    std::unordered_map<std::string, GLuint> m_textureUnits;
};

// Конкретные типы материалов
class BasicMaterial : public Material {
public:
    BasicMaterial();
    ~BasicMaterial() override = default;
    
    void Apply(ShaderProgram* shader) override;
    void BindTextures() override;
    std::unique_ptr<Material> Clone() const override;
    
    // Специфичные свойства (удобные обертки)
    void SetColor(const glm::vec4& color);
    glm::vec4 GetColor() const;
    
    void SetTexture(Texture* texture);
    Texture* GetTexture() const;
    
    void SetUseLighting(bool use);
    bool GetUseLighting() const;
    
private:
    // НЕТ дублирования данных! Все хранится в базовых m_*Params
};

class PBRMaterial : public Material {
public:
    PBRMaterial();
    ~PBRMaterial() override = default;
    
    void Apply(ShaderProgram* shader) override;
    void BindTextures() override;
    std::unique_ptr<Material> Clone() const override;
    
    // PBR свойства (удобные обертки)
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
    // Только указатели на текстуры (данные в m_textureParams)
    Texture* m_albedoMap = nullptr;
    Texture* m_normalMap = nullptr;
    Texture* m_metallicRoughnessMap = nullptr;
    Texture* m_aoMap = nullptr;
    Texture* m_emissionMap = nullptr;
};

// Material Instance (для переопределения параметров)
class MaterialInstance : public Material {
public:
    MaterialInstance(Material* baseMaterial = nullptr);
    ~MaterialInstance() override = default;
    
    void Apply(ShaderProgram* shader) override;
    void BindTextures() override;
    std::unique_ptr<Material> Clone() const override;
    
    // Переопределение параметров
    void OverrideFloat(const std::string& name, float value);
    void OverrideVec3(const std::string& name, const glm::vec3& value);
    void OverrideVec4(const std::string& name, const glm::vec4& value);
    void OverrideTexture(const std::string& name, Texture* texture);
    
    // Управление базовым материалом
    void SetBaseMaterial(Material* material);
    Material* GetBaseMaterial() const;
    
private:
    Material* m_baseMaterial = nullptr;
};

} // namespace ogle