// src/render/MaterialController.h
#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>

#include "log/Logger.h"
#include "render/material/Material.h"
#include "render/material/JsonMaterialSerializer.h"
#include "render/ShaderController.h"

namespace ogle {

// Публичный интерфейс (синглтон)
class MaterialController {
public:
    static MaterialController& Get();

    ~MaterialController();

    // === Создание материалов ===
    Material* CreateMaterial(const std::string& name, MaterialType type);
    Material* CreateMaterialFromShader(const std::string& name,
                                      const std::string& shaderName);

    // === Управление материалами ===
    Material* GetMaterial(const std::string& name);
    bool HasMaterial(const std::string& name) const;
    void RemoveMaterial(const std::string& name);

    // === Material Instances ===
    Material* CreateInstance(const std::string& name,
                            const std::string& baseMaterialName);
    Material* CreateInstance(Material* baseMaterial,
                            const std::string& instanceName = "");

    // === Предопределенные материалы ===
    enum class Builtin {
        DefaultWhite,
        DefaultRed,
        DefaultGreen,
        DefaultBlue,
        DefaultCheckerboard,
        DefaultPBRMetal,
        DefaultPBRPlastic,
        DefaultPBREmissive
    };

    Material* GetBuiltin(Builtin type);
    void PreloadBuiltinMaterials();

    // === Утилиты ===
    void Clear();
    size_t GetMaterialCount() const;

    // === Поиск и фильтрация ===
    std::vector<std::string> GetMaterialNames() const;
    std::vector<Material*> GetMaterialsByShader(const std::string& shaderName) const;

    // === Отладка ===
    void PrintDebugInfo() const;

    // === Сохранение/Загрузка ===
    bool SaveMaterialToFile(const std::string& name, const std::string& filepath);
    Material* LoadMaterialFromFile(const std::string& filepath,
                                  const std::string& materialName = "");

private:
    MaterialController();

    // Вспомогательные методы
    std::string GenerateUniqueName(const std::string& baseName) const;
    void SetupDefaultRenderState(Material* material);
    std::string GetBuiltinName(Builtin type) const;
    void CreateBuiltinMaterial(Builtin type);

    // Хранилище
    std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;

    // Статистика
    struct Statistics {
        size_t totalMaterials = 0;
        size_t basicMaterials = 0;
        size_t pbrMaterials = 0;
        size_t materialInstances = 0;
        size_t builtinMaterials = 0;
    } m_stats;

    // Запрет копирования
    MaterialController(const MaterialController&) = delete;
    MaterialController& operator=(const MaterialController&) = delete;
};

} // namespace ogle
