// src/render/MaterialController.h
#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "log/Logger.h"

namespace ogle {

// Предварительные объявления
class Material;
class MaterialControllerImpl;
enum class MaterialType;

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
    
    std::unique_ptr<MaterialControllerImpl> m_impl;
    
    // Запрет копирования
    MaterialController(const MaterialController&) = delete;
    MaterialController& operator=(const MaterialController&) = delete;
};

} // namespace ogle