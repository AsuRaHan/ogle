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


// Предварительные объявления
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

    // Абстрактный базовый класс материала
    class Material {
    public:
        virtual ~Material() = default;

        // Чистые виртуальные методы
        virtual void Apply(ShaderProgram* shader) = 0;
        virtual void BindTextures() = 0;
        virtual std::unique_ptr<Material> Clone() const = 0;
        virtual MaterialType GetType() const = 0;

        // ==================== ОБЩИЕ ВИРТУАЛЬНЫЕ МЕТОДЫ ====================
        // Эти методы предоставляют общий интерфейс для ВСЕХ материалов

        // Цвет (для BasicMaterial и других простых материалов)
        virtual void SetColor(const glm::vec4& color) {
            // Базовая реализация - ничего не делает
            // Переопределяется в производных классах
        }

        virtual glm::vec4 GetColor() const {
            return glm::vec4(0.0f);
        }

        // Освещение
        virtual void SetUseLighting(bool use) {
            // Базовая реализация
        }

        virtual bool GetUseLighting() const {
            return false;
        }

        // Текстура (удобная версия без указания имени параметра)
        virtual void SetTexture(Texture* texture) {
            // Базовая реализация - вызывает версию с именем
            SetTexture("uTexture", texture);
        }

        virtual Texture* GetTexture() const {
            return GetTexture("uTexture");
        }

        // PBR свойства (для PBRMaterial)
        virtual void SetAlbedo(const glm::vec4& albedo) {}
        virtual glm::vec4 GetAlbedo() const { return glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); }

        virtual void SetMetallic(float metallic) {}
        virtual float GetMetallic() const { return 0.0f; }

        virtual void SetRoughness(float roughness) {}
        virtual float GetRoughness() const { return 0.5f; }

        virtual void SetAO(float ao) {}
        virtual float GetAO() const { return 1.0f; }

        virtual void SetEmission(const glm::vec3& emission) {}
        virtual glm::vec3 GetEmission() const { return glm::vec3(0.0f, 0.0f, 0.0f); }
        // ==================== КОНЕЦ ОБЩИХ МЕТОДОВ ====================

        // Общие свойства (не виртуальные)
        std::string GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        ShaderProgram* GetShader() const { return m_shader; }
        void SetShader(ShaderProgram* shader) { m_shader = shader; }

        const RenderState& GetRenderState() const { return m_renderState; }
        RenderState& GetRenderState() { return m_renderState; }
        void SetRenderState(const RenderState& state) { m_renderState = state; }

        // Параметры материала (универсальные)
        virtual void SetFloat(const std::string& name, float value) { m_floatParams[name] = value; }
        virtual void SetVec3(const std::string& name, const glm::vec3& value) { m_vec3Params[name] = value; }
        virtual void SetVec4(const std::string& name, const glm::vec4& value) { m_vec4Params[name] = value; }
        virtual void SetInt(const std::string& name, int value) { m_intParams[name] = value; }
        virtual void SetTexture(const std::string& name, Texture* texture) {
            m_textureParams[name] = texture;
            SetInt(name + "_enabled", texture ? 1 : 0);
        }

        virtual float GetFloat(const std::string& name, float defaultValue = 0.0f) const {
            auto it = m_floatParams.find(name);
            return it != m_floatParams.end() ? it->second : defaultValue;
        }

        virtual glm::vec3 GetVec3(const std::string& name, const glm::vec3& defaultValue = glm::vec3(0)) const {
            auto it = m_vec3Params.find(name);
            return it != m_vec3Params.end() ? it->second : defaultValue;
        }

        virtual glm::vec4 GetVec4(const std::string& name, const glm::vec4& defaultValue = glm::vec4(0)) const {
            auto it = m_vec4Params.find(name);
            return it != m_vec4Params.end() ? it->second : defaultValue;
        }

        virtual int GetInt(const std::string& name, int defaultValue = 0) const {
            auto it = m_intParams.find(name);
            return it != m_intParams.end() ? it->second : defaultValue;
        }

        virtual Texture* GetTexture(const std::string& name) const {
            auto it = m_textureParams.find(name);
            return it != m_textureParams.end() ? it->second : nullptr;
        }

        // Применение всех параметров к шейдеру
        virtual void ApplyParameters(ShaderProgram* shader) const;

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
        MaterialType GetType() const override { return MaterialType::Basic; }

        // ==================== ПЕРЕОПРЕДЕЛЕННЫЕ ВИРТУАЛЬНЫЕ МЕТОДЫ ====================
        void SetColor(const glm::vec4& color) override {
            SetVec3("uColor", glm::vec3(color));
        }

        glm::vec4 GetColor() const override {
            // Возвращаем vec4, добавляя альфа=1.0
            glm::vec3 colorVec3 = GetVec3("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
            return glm::vec4(colorVec3, 1.0f);
        }

        void SetUseLighting(bool use) override {
            SetInt("uUseLighting", use ? 1 : 0);
        }

        bool GetUseLighting() const override {
            return GetInt("uUseLighting", 1) != 0;
        }

        // ОСОБОЕ ВНИМАНИЕ: эта версия SetTexture устанавливает флаг uUseTexture
        void SetTexture(Texture* texture) override {
            // Сначала вызываем базовую версию с именем параметра
            Material::SetTexture("uTexture", texture);
            // Затем устанавливаем флаг использования текстуры
            SetInt("uUseTexture", texture ? 1 : 0);
        }

        Texture* GetTexture() const override {
            return Material::GetTexture("uTexture");
        }
        // ==================== КОНЕЦ ПЕРЕОПРЕДЕЛЕННЫХ МЕТОДОВ ====================

        // Специфичные свойства (удобные обертки - оставляем для обратной совместимости)
        // Эти методы теперь просто вызывают переопределенные виртуальные методы
        void SetColorWrapped(const glm::vec4& color) { SetColor(color); }
        glm::vec4 GetColorWrapped() const { return GetColor(); }

        void SetTextureWrapped(Texture* texture) { SetTexture(texture); }
        Texture* GetTextureWrapped() const { return GetTexture(); }

        void SetUseLightingWrapped(bool use) { SetUseLighting(use); }
        bool GetUseLightingWrapped() const { return GetUseLighting(); }
    };

    class PBRMaterial : public Material {
    public:
        PBRMaterial();
        ~PBRMaterial() override = default;

        void Apply(ShaderProgram* shader) override;
        void BindTextures() override;
        std::unique_ptr<Material> Clone() const override;
        MaterialType GetType() const override { return MaterialType::PBR; }

        // ==================== ПЕРЕОПРЕДЕЛЕННЫЕ ВИРТУАЛЬНЫЕ МЕТОДЫ ====================
        void SetAlbedo(const glm::vec4& albedo) override {
            SetVec4("uAlbedo", albedo);
        }

        glm::vec4 GetAlbedo() const override {
            return GetVec4("uAlbedo", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        }

        void SetMetallic(float metallic) override {
            SetFloat("uMetallic", metallic);
        }

        float GetMetallic() const override {
            return GetFloat("uMetallic", 0.0f);
        }

        void SetRoughness(float roughness) override {
            SetFloat("uRoughness", roughness);
        }

        float GetRoughness() const override {
            return GetFloat("uRoughness", 0.5f);
        }

        void SetAO(float ao) override {
            SetFloat("uAO", ao);
        }

        float GetAO() const override {
            return GetFloat("uAO", 1.0f);
        }

        void SetEmission(const glm::vec3& emission) override {
            SetVec3("uEmission", emission);
        }

        glm::vec3 GetEmission() const override {
            return GetVec3("uEmission", glm::vec3(0.0f, 0.0f, 0.0f));
        }
        // ==================== КОНЕЦ ПЕРЕОПРЕДЕЛЕННЫХ МЕТОДОВ ====================

        // PBR свойства (удобные обертки)
        void SetAlbedoWrapped(const glm::vec4& albedo) { SetAlbedo(albedo); }
        void SetMetallicWrapped(float metallic) { SetMetallic(metallic); }
        void SetRoughnessWrapped(float roughness) { SetRoughness(roughness); }
        void SetAOWrapped(float ao) { SetAO(ao); }
        void SetEmissionWrapped(const glm::vec3& emission) { SetEmission(emission); }

        glm::vec4 GetAlbedoWrapped() const { return GetAlbedo(); }
        float GetMetallicWrapped() const { return GetMetallic(); }
        float GetRoughnessWrapped() const { return GetRoughness(); }
        float GetAOWrapped() const { return GetAO(); }
        glm::vec3 GetEmissionWrapped() const { return GetEmission(); }

        // Методы для текстур (специфичные для PBRMaterial)
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
        Texture* m_albedoMap = nullptr;
        Texture* m_normalMap = nullptr;
        Texture* m_metallicRoughnessMap = nullptr;
        Texture* m_aoMap = nullptr;
        Texture* m_emissionMap = nullptr;
    };

// Material Instance (декоратор для переопределения параметров)
    class MaterialInstance : public Material {
    public:
        MaterialInstance(Material* baseMaterial = nullptr);
        ~MaterialInstance() override = default;

        void Apply(ShaderProgram* shader) override;
        void BindTextures() override;
        std::unique_ptr<Material> Clone() const override;
        MaterialType GetType() const override {
            return m_baseMaterial ? m_baseMaterial->GetType() : MaterialType::Custom;
        }

        // ==================== ПЕРЕОПРЕДЕЛЕННЫЕ ВИРТУАЛЬНЫЕ МЕТОДЫ ====================
        // Цвет и освещение
        void SetColor(const glm::vec4& color) override {
            Material::SetColor(color); // Сохраняем в своих параметрах
        }

        glm::vec4 GetColor() const override {
            // Если переопределен - берем наш, иначе из базового материала
            if (auto it = m_vec4Params.find("uColor"); it != m_vec4Params.end()) {
                return it->second;
            }
            return m_baseMaterial ? m_baseMaterial->GetColor() : glm::vec4(0.0f);
        }

        void SetUseLighting(bool use) override {
            SetInt("uUseLighting", use ? 1 : 0);
        }

        bool GetUseLighting() const override {
            // Если переопределен - берем наш, иначе из базового материала
            if (auto it = m_intParams.find("uUseLighting"); it != m_intParams.end()) {
                return it->second != 0;
            }
            return m_baseMaterial ? m_baseMaterial->GetUseLighting() : false;
        }

        // Текстура (удобная версия без имени)
        using Material::SetTexture;

        Texture* GetTexture() const override {
            // Если переопределен - берем наш, иначе из базового материала
            if (auto it = m_textureParams.find("uTexture"); it != m_textureParams.end()) {
                return it->second;
            }
            return m_baseMaterial ? m_baseMaterial->GetTexture() : nullptr;
        }

        // PBR методы
        void SetAlbedo(const glm::vec4& albedo) override {
            SetVec4("uAlbedo", albedo);
        }

        glm::vec4 GetAlbedo() const override {
            if (auto it = m_vec4Params.find("uAlbedo"); it != m_vec4Params.end()) {
                return it->second;
            }
            return m_baseMaterial ? m_baseMaterial->GetAlbedo() : glm::vec4(0.0f);
        }

        void SetMetallic(float metallic) override {
            SetFloat("uMetallic", metallic);
        }

        float GetMetallic() const override {
            if (auto it = m_floatParams.find("uMetallic"); it != m_floatParams.end()) {
                return it->second;
            }
            return m_baseMaterial ? m_baseMaterial->GetMetallic() : 0.0f;
        }

        void SetRoughness(float roughness) override {
            SetFloat("uRoughness", roughness);
        }

        float GetRoughness() const override {
            if (auto it = m_floatParams.find("uRoughness"); it != m_floatParams.end()) {
                return it->second;
            }
            return m_baseMaterial ? m_baseMaterial->GetRoughness() : 0.5f;
        }
        // ==================== КОНЕЦ ПЕРЕОПРЕДЕЛЕННЫХ МЕТОДОВ ====================
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