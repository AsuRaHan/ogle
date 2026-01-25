// src/render/material/MaterialControllerImpl.h
#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "log/Logger.h"
#include "Material.h"

namespace ogle {

//enum class MaterialType {
//    Basic,
//    PBR,
//    Unlit,
//    Custom
//};

// Встроенные материалы
enum class BuiltinMaterial {
    DefaultWhite,
    DefaultRed,
    DefaultGreen,
    DefaultBlue,
    DefaultCheckerboard,
    DefaultPBRMetal,
    DefaultPBRPlastic,
    DefaultPBREmissive
};

// Приватная реализация
class MaterialControllerImpl {
public:
    MaterialControllerImpl();
    ~MaterialControllerImpl();
    
    // Основные методы
    Material* CreateMaterial(const std::string& name, MaterialType type);
    Material* CreateMaterialFromShader(const std::string& name, 
                                      const std::string& shaderName);
    
    Material* GetMaterial(const std::string& name);
    bool HasMaterial(const std::string& name) const;
    void RemoveMaterial(const std::string& name);
    
    // Instances
    Material* CreateInstance(const std::string& name, 
                            const std::string& baseMaterialName);
    Material* CreateInstance(Material* baseMaterial, 
                            const std::string& instanceName = "");
    
    // Builtin материалы
    Material* GetBuiltin(BuiltinMaterial type);
    void PreloadBuiltinMaterials();
    
    // Управление
    void Clear();
    size_t GetMaterialCount() const;
    
    // Утилиты
    std::vector<std::string> GetMaterialNames() const;
    std::vector<Material*> GetMaterialsByShader(const std::string& shaderName) const;
    
    // Отладка
    void PrintDebugInfo() const;
    
    // Сохранение/загрузка
    bool SaveMaterialToFile(const std::string& name, const std::string& filepath);
    Material* LoadMaterialFromFile(const std::string& filepath, 
                                  const std::string& materialName = "");
    
private:
    // Вспомогательные методы
    std::string GenerateUniqueName(const std::string& baseName) const;
    void SetupDefaultRenderState(Material* material);
    
    // Builtin материалы
    std::string GetBuiltinName(BuiltinMaterial type) const;
    void CreateBuiltinMaterial(BuiltinMaterial type);
    
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
};

} // namespace ogle