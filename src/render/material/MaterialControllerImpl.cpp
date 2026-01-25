// src/render/material/MaterialControllerImpl.cpp
#include "MaterialControllerImpl.h"
#include "Material.h"
#include "render/ShaderController.h"
#include <sstream>
#include <iomanip>

namespace ogle {

MaterialControllerImpl::MaterialControllerImpl() {
    Logger::Info("MaterialControllerImpl initialized");
    PreloadBuiltinMaterials();
}

MaterialControllerImpl::~MaterialControllerImpl() {
    Clear();
    Logger::Info("MaterialControllerImpl shutdown");
}

Material* MaterialControllerImpl::CreateMaterial(const std::string& name, MaterialType type) {
    // Проверяем уникальность имени
    if (HasMaterial(name)) {
        Logger::Warning("Material already exists: " + name);
        return GetMaterial(name);
    }
    
    std::unique_ptr<Material> material;
    
    // Создаем материал нужного типа
    switch (type) {
        case MaterialType::Basic: {
            auto basic = std::make_unique<BasicMaterial>();
            basic->SetName(name);
            
            // Связываем с базовым шейдером
            auto& shaderCtrl = ShaderController::Get();
            auto shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
            if (shader) {
                basic->SetShader(shader.get());
            }
            
            material = std::move(basic);
            m_stats.basicMaterials++;
            break;
        }
        
        case MaterialType::PBR: {
            auto pbr = std::make_unique<PBRMaterial>();
            pbr->SetName(name);
            
            // TODO: Создать/получить PBR шейдер
            // Пока используем BasicColor
            auto& shaderCtrl = ShaderController::Get();
            auto shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
            if (shader) {
                pbr->SetShader(shader.get());
            }
            
            material = std::move(pbr);
            m_stats.pbrMaterials++;
            break;
        }
        
        default:
            Logger::Error("Unsupported material type");
            return nullptr;
    }
    
    if (!material) {
        Logger::Error("Failed to create material: " + name);
        return nullptr;
    }
    
    // Настраиваем состояние рендеринга
    SetupDefaultRenderState(material.get());
    
    // Сохраняем
    Material* ptr = material.get();
    m_materials[name] = std::move(material);
    m_stats.totalMaterials++;
    
    Logger::Info("Material created: " + name + 
                " (Type: " + (type == MaterialType::Basic ? "Basic" : "PBR") + ")");
    
    return ptr;
}

Material* MaterialControllerImpl::CreateMaterialFromShader(const std::string& name, 
                                                          const std::string& shaderName) {
    // Получаем шейдер
    auto& shaderCtrl = ShaderController::Get();
    auto shader = shaderCtrl.GetProgram(shaderName);
    if (!shader) {
        Logger::Error("Shader not found: " + shaderName);
        return nullptr;
    }
    
    // Создаем базовый материал
    auto material = CreateMaterial(name, MaterialType::Basic);
    if (!material) return nullptr;
    
    // Привязываем шейдер
    material->SetShader(shader.get());
    
    Logger::Info("Material created from shader: " + name + " -> " + shaderName);
    return material;
}

Material* MaterialControllerImpl::GetMaterial(const std::string& name) {
    auto it = m_materials.find(name);
    return it != m_materials.end() ? it->second.get() : nullptr;
}

bool MaterialControllerImpl::HasMaterial(const std::string& name) const {
    return m_materials.find(name) != m_materials.end();
}

void MaterialControllerImpl::RemoveMaterial(const std::string& name) {
    auto it = m_materials.find(name);
    if (it != m_materials.end()) {
        // Обновляем статистику
        auto* material = it->second.get();
        if (dynamic_cast<BasicMaterial*>(material)) {
            m_stats.basicMaterials--;
        } else if (dynamic_cast<PBRMaterial*>(material)) {
            m_stats.pbrMaterials--;
        } else if (dynamic_cast<MaterialInstance*>(material)) {
            m_stats.materialInstances--;
        }
        
        m_materials.erase(it);
        m_stats.totalMaterials--;
        Logger::Info("Material removed: " + name);
    }
}

Material* MaterialControllerImpl::CreateInstance(const std::string& name, 
                                                const std::string& baseMaterialName) {
    // Находим базовый материал
    auto* baseMaterial = GetMaterial(baseMaterialName);
    if (!baseMaterial) {
        Logger::Error("Base material not found: " + baseMaterialName);
        return nullptr;
    }
    
    return CreateInstance(baseMaterial, name);
}

Material* MaterialControllerImpl::CreateInstance(Material* baseMaterial, 
                                                const std::string& instanceName) {
    if (!baseMaterial) {
        Logger::Error("Cannot create instance from null base material");
        return nullptr;
    }
    
    // Генерируем имя если не указано
    std::string name = instanceName;
    if (name.empty()) {
        name = GenerateUniqueName(baseMaterial->GetName() + "_Instance");
    }
    
    // Проверяем уникальность
    if (HasMaterial(name)) {
        Logger::Warning("Material instance already exists: " + name);
        return GetMaterial(name);
    }
    
    // Создаем инстанс
    auto instance = std::make_unique<MaterialInstance>(baseMaterial);
    instance->SetName(name);
    
    // Копируем шейдер из базового материала
    instance->SetShader(baseMaterial->GetShader());
    
    // Сохраняем
    Material* ptr = instance.get();
    m_materials[name] = std::move(instance);
    
    m_stats.totalMaterials++;
    m_stats.materialInstances++;
    
    Logger::Info("Material instance created: " + name + 
                " (Base: " + baseMaterial->GetName() + ")");
    
    return ptr;
}

Material* MaterialControllerImpl::GetBuiltin(BuiltinMaterial type) {
    std::string name = GetBuiltinName(type);
    
    // Проверяем, есть ли уже
    auto* existing = GetMaterial(name);
    if (existing) {
        return existing;
    }
    
    // Создаем если нет
    CreateBuiltinMaterial(type);
    return GetMaterial(name);
}

void MaterialControllerImpl::PreloadBuiltinMaterials() {
    Logger::Info("Preloading builtin materials...");
    
    // Создаем все встроенные материалы
    for (int i = 0; i <= static_cast<int>(BuiltinMaterial::DefaultPBREmissive); i++) {
        CreateBuiltinMaterial(static_cast<BuiltinMaterial>(i));
    }
    
    Logger::Info("Builtin materials preloaded");
}

// Вспомогательные методы
std::string MaterialControllerImpl::GenerateUniqueName(const std::string& baseName) const {
    std::string name = baseName;
    int counter = 1;
    
    while (HasMaterial(name)) {
        std::stringstream ss;
        ss << baseName << "_" << std::setw(3) << std::setfill('0') << counter++;
        name = ss.str();
    }
    
    return name;
}

void MaterialControllerImpl::SetupDefaultRenderState(Material* material) {
    if (!material) return;
    
    auto& state = material->GetRenderState();
    state.depthTest = true;
    state.depthWrite = true;
    state.depthFunc = RenderState::Less;
    state.cullFace = true;
    state.cullMode = RenderState::Back;
    state.blend = false;
    state.wireframe = false;
    state.lineWidth = 1.0f;
}

std::string MaterialControllerImpl::GetBuiltinName(BuiltinMaterial type) const {
    switch (type) {
        case BuiltinMaterial::DefaultWhite: return "Builtin_White";
        case BuiltinMaterial::DefaultRed: return "Builtin_Red";
        case BuiltinMaterial::DefaultGreen: return "Builtin_Green";
        case BuiltinMaterial::DefaultBlue: return "Builtin_Blue";
        case BuiltinMaterial::DefaultCheckerboard: return "Builtin_Checkerboard";
        case BuiltinMaterial::DefaultPBRMetal: return "Builtin_PBR_Metal";
        case BuiltinMaterial::DefaultPBRPlastic: return "Builtin_PBR_Plastic";
        case BuiltinMaterial::DefaultPBREmissive: return "Builtin_PBR_Emissive";
        default: return "Builtin_Unknown";
    }
}

void MaterialControllerImpl::CreateBuiltinMaterial(BuiltinMaterial type) {
    std::string name = GetBuiltinName(type);
    
    // Проверяем, не создан ли уже
    if (HasMaterial(name)) {
        return;
    }
    
    auto& shaderCtrl = ShaderController::Get();
    
    switch (type) {
        case BuiltinMaterial::DefaultWhite: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) {
                material->SetVec4("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                m_stats.builtinMaterials++;
            }
            break;
        }
        
        case BuiltinMaterial::DefaultRed: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) {
                material->SetVec4("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
                m_stats.builtinMaterials++;
            }
            break;
        }
        
        case BuiltinMaterial::DefaultGreen: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) {
                material->SetVec4("uColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                m_stats.builtinMaterials++;
            }
            break;
        }
        
        case BuiltinMaterial::DefaultBlue: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) {
                material->SetVec4("uColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
                m_stats.builtinMaterials++;
            }
            break;
        }
        
        // Остальные материалы можно доделать позже
        default: {
            // Создаем базовый белый как заглушку
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) {
                material->SetVec4("uColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
                m_stats.builtinMaterials++;
            }
            break;
        }
    }
    
    Logger::Debug("Builtin material created: " + name);
}

// Остальные методы (пока заглушки)
void MaterialControllerImpl::Clear() {
    m_materials.clear();
    m_stats = Statistics();
    Logger::Info("All materials cleared");
}

size_t MaterialControllerImpl::GetMaterialCount() const {
    return m_materials.size();
}

std::vector<std::string> MaterialControllerImpl::GetMaterialNames() const {
    std::vector<std::string> names;
    names.reserve(m_materials.size());
    for (const auto& pair : m_materials) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<Material*> MaterialControllerImpl::GetMaterialsByShader(const std::string& shaderName) const {
    std::vector<Material*> result;
    // TODO: Реализовать поиск по шейдеру
    return result;
}

void MaterialControllerImpl::PrintDebugInfo() const {
    Logger::Info("=== MaterialControllerImpl Debug Info ===");
    Logger::Info("Total materials: " + std::to_string(m_stats.totalMaterials));
    Logger::Info("Basic materials: " + std::to_string(m_stats.basicMaterials));
    Logger::Info("PBR materials: " + std::to_string(m_stats.pbrMaterials));
    Logger::Info("Material instances: " + std::to_string(m_stats.materialInstances));
    Logger::Info("Builtin materials: " + std::to_string(m_stats.builtinMaterials));
    
    Logger::Info("Loaded materials:");
    for (const auto& pair : m_materials) {
        auto* material = pair.second.get();
        std::string type = "Unknown";
        if (dynamic_cast<BasicMaterial*>(material)) type = "Basic";
        else if (dynamic_cast<PBRMaterial*>(material)) type = "PBR";
        else if (dynamic_cast<MaterialInstance*>(material)) type = "Instance";
        
        Logger::Info("  - " + pair.first + " (" + type + ")");
    }
}

bool MaterialControllerImpl::SaveMaterialToFile(const std::string& name, const std::string& filepath) {
    // TODO: Реализовать сохранение в JSON
    Logger::Warning("SaveMaterialToFile not implemented yet");
    return false;
}

Material* MaterialControllerImpl::LoadMaterialFromFile(const std::string& filepath, 
                                                      const std::string& materialName) {
    // TODO: Реализовать загрузку из JSON
    Logger::Warning("LoadMaterialFromFile not implemented yet");
    return nullptr;
}

} // namespace ogle