// src/render/MaterialController.cpp
#include "MaterialController.h"
#include "render/material/MaterialControllerImpl.h"
#include "render/ShaderController.h"

namespace ogle {

MaterialController& MaterialController::Get() {
    static MaterialController instance;
    return instance;
}

MaterialController::MaterialController()
    : m_impl(std::make_unique<MaterialControllerImpl>()) {
    Logger::Info("MaterialController initialized");
}

MaterialController::~MaterialController() {
    Logger::Info("MaterialController shutdown");
}

// Делегируем все вызовы реализации
Material* MaterialController::CreateMaterial(const std::string& name, MaterialType type) {
    return m_impl->CreateMaterial(name, type);
}

Material* MaterialController::CreateMaterialFromShader(const std::string& name, 
                                                      const std::string& shaderName) {
    return m_impl->CreateMaterialFromShader(name, shaderName);
}

Material* MaterialController::GetMaterial(const std::string& name) {
    return m_impl->GetMaterial(name);
}

bool MaterialController::HasMaterial(const std::string& name) const {
    return m_impl->HasMaterial(name);
}

void MaterialController::RemoveMaterial(const std::string& name) {
    m_impl->RemoveMaterial(name);
}

Material* MaterialController::CreateInstance(const std::string& name, 
                                            const std::string& baseMaterialName) {
    return m_impl->CreateInstance(name, baseMaterialName);
}

Material* MaterialController::CreateInstance(Material* baseMaterial, 
                                            const std::string& instanceName) {
    return m_impl->CreateInstance(baseMaterial, instanceName);
}

Material* MaterialController::GetBuiltin(Builtin type) {
    // Конвертируем Builtin -> BuiltinMaterial
    return m_impl->GetBuiltin(static_cast<BuiltinMaterial>(type));
}

void MaterialController::PreloadBuiltinMaterials() {
    m_impl->PreloadBuiltinMaterials();
}

void MaterialController::Clear() {
    m_impl->Clear();
}

size_t MaterialController::GetMaterialCount() const {
    return m_impl->GetMaterialCount();
}

std::vector<std::string> MaterialController::GetMaterialNames() const {
    return m_impl->GetMaterialNames();
}

std::vector<Material*> MaterialController::GetMaterialsByShader(const std::string& shaderName) const {
    return m_impl->GetMaterialsByShader(shaderName);
}

void MaterialController::PrintDebugInfo() const {
    m_impl->PrintDebugInfo();
}

bool MaterialController::SaveMaterialToFile(const std::string& name, const std::string& filepath) {
    return m_impl->SaveMaterialToFile(name, filepath);
}

Material* MaterialController::LoadMaterialFromFile(const std::string& filepath, 
                                                  const std::string& materialName) {
    return m_impl->LoadMaterialFromFile(filepath, materialName);
}

} // namespace ogle