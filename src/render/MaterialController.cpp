// src/render/MaterialController.cpp
#include "MaterialController.h"

namespace ogle {

MaterialController& MaterialController::Get() {
    static MaterialController instance;
    return instance;
}

MaterialController::MaterialController() {
    Logger::Info("MaterialController initialized");
    PreloadBuiltinMaterials();
}

MaterialController::~MaterialController() {
    Clear();
    Logger::Info("MaterialController shutdown");
}

Material* MaterialController::CreateMaterial(const std::string& name, MaterialType type) {
    if (HasMaterial(name)) {
        Logger::Warning("Material already exists: " + name);
        return GetMaterial(name);
    }

    std::unique_ptr<Material> material;

    switch (type) {
        case MaterialType::Basic: {
            auto basic = std::make_unique<BasicMaterial>();
            basic->SetName(name);
            auto& shaderCtrl = ShaderController::Get();
            auto shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
            if (shader) basic->SetShader(shader.get());
            material = std::move(basic);
            m_stats.basicMaterials++;
            break;
        }
        case MaterialType::PBR: {
            auto pbr = std::make_unique<PBRMaterial>();
            pbr->SetName(name);
            auto& shaderCtrl = ShaderController::Get();
            auto shader = shaderCtrl.GetBuiltin(ShaderController::Builtin::BasicColor);
            if (shader) pbr->SetShader(shader.get());
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

    SetupDefaultRenderState(material.get());

    Material* ptr = material.get();
    m_materials[name] = std::move(material);
    m_stats.totalMaterials++;

    Logger::Info("Material created: " + name);
    return ptr;
}

Material* MaterialController::CreateMaterialFromShader(const std::string& name,
                                                      const std::string& shaderName) {
    auto& shaderCtrl = ShaderController::Get();
    auto shader = shaderCtrl.GetProgram(shaderName);
    if (!shader) {
        Logger::Error("Shader not found: " + shaderName);
        return nullptr;
    }

    auto material = CreateMaterial(name, MaterialType::Basic);
    if (!material) return nullptr;
    material->SetShader(shader.get());
    Logger::Info("Material created from shader: " + name + " -> " + shaderName);
    return material;
}

Material* MaterialController::GetMaterial(const std::string& name) {
    auto it = m_materials.find(name);
    return it != m_materials.end() ? it->second.get() : nullptr;
}

bool MaterialController::HasMaterial(const std::string& name) const {
    return m_materials.find(name) != m_materials.end();
}

void MaterialController::RemoveMaterial(const std::string& name) {
    auto it = m_materials.find(name);
    if (it != m_materials.end()) {
        auto* material = it->second.get();
        if (dynamic_cast<BasicMaterial*>(material)) m_stats.basicMaterials--;
        else if (dynamic_cast<PBRMaterial*>(material)) m_stats.pbrMaterials--;
        else if (dynamic_cast<MaterialInstance*>(material)) m_stats.materialInstances--;
        m_materials.erase(it);
        m_stats.totalMaterials--;
        Logger::Info("Material removed: " + name);
    }
}

Material* MaterialController::CreateInstance(const std::string& name,
                                            const std::string& baseMaterialName) {
    auto* baseMaterial = GetMaterial(baseMaterialName);
    if (!baseMaterial) {
        Logger::Error("Base material not found: " + baseMaterialName);
        return nullptr;
    }
    return CreateInstance(baseMaterial, name);
}

Material* MaterialController::CreateInstance(Material* baseMaterial,
                                            const std::string& instanceName) {
    if (!baseMaterial) {
        Logger::Error("Cannot create instance from null base material");
        return nullptr;
    }

    std::string name = instanceName;
    if (name.empty()) name = GenerateUniqueName(baseMaterial->GetName() + "_Instance");

    if (HasMaterial(name)) {
        Logger::Warning("Material instance already exists: " + name);
        return GetMaterial(name);
    }

    auto instance = std::make_unique<MaterialInstance>(baseMaterial);
    instance->SetName(name);
    instance->SetShader(baseMaterial->GetShader());
    Material* ptr = instance.get();
    m_materials[name] = std::move(instance);
    m_stats.totalMaterials++;
    m_stats.materialInstances++;
    Logger::Info("Material instance created: " + name);
    return ptr;
}

Material* MaterialController::GetBuiltin(Builtin type) {
    std::string name = GetBuiltinName(type);
    auto* existing = GetMaterial(name);
    if (existing) return existing;
    CreateBuiltinMaterial(type);
    return GetMaterial(name);
}

void MaterialController::PreloadBuiltinMaterials() {
    Logger::Info("Preloading builtin materials...");
    for (int i = 0; i <= static_cast<int>(Builtin::DefaultPBREmissive); i++) {
        CreateBuiltinMaterial(static_cast<Builtin>(i));
    }
    Logger::Info("Builtin materials preloaded");
}

std::string MaterialController::GenerateUniqueName(const std::string& baseName) const {
    std::string name = baseName;
    int counter = 1;
    while (HasMaterial(name)) {
        std::stringstream ss;
        ss << baseName << "_" << std::setw(3) << std::setfill('0') << counter++;
        name = ss.str();
    }
    return name;
}

void MaterialController::SetupDefaultRenderState(Material* material) {
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

std::string MaterialController::GetBuiltinName(Builtin type) const {
    switch (type) {
        case Builtin::DefaultWhite: return "Builtin_White";
        case Builtin::DefaultRed: return "Builtin_Red";
        case Builtin::DefaultGreen: return "Builtin_Green";
        case Builtin::DefaultBlue: return "Builtin_Blue";
        case Builtin::DefaultCheckerboard: return "Builtin_Checkerboard";
        case Builtin::DefaultPBRMetal: return "Builtin_PBR_Metal";
        case Builtin::DefaultPBRPlastic: return "Builtin_PBR_Plastic";
        case Builtin::DefaultPBREmissive: return "Builtin_PBR_Emissive";
        default: return "Builtin_Unknown";
    }
}

void MaterialController::CreateBuiltinMaterial(Builtin type) {
    std::string name = GetBuiltinName(type);
    if (HasMaterial(name)) return;
    switch (type) {
        case Builtin::DefaultWhite: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) { material->SetVec4("uColor", glm::vec4(1.0f)); m_stats.builtinMaterials++; }
            break;
        }
        case Builtin::DefaultRed: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) { material->SetVec4("uColor", glm::vec4(1.0f,0.0f,0.0f,1.0f)); m_stats.builtinMaterials++; }
            break;
        }
        case Builtin::DefaultGreen: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) { material->SetVec4("uColor", glm::vec4(0.0f,1.0f,0.0f,1.0f)); m_stats.builtinMaterials++; }
            break;
        }
        case Builtin::DefaultBlue: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) { material->SetVec4("uColor", glm::vec4(0.0f,0.0f,1.0f,1.0f)); m_stats.builtinMaterials++; }
            break;
        }
        default: {
            auto* material = CreateMaterial(name, MaterialType::Basic);
            if (material) { material->SetVec4("uColor", glm::vec4(0.8f,0.8f,0.8f,1.0f)); m_stats.builtinMaterials++; }
            break;
        }
    }
    Logger::Debug("Builtin material created: " + name);
}

void MaterialController::Clear() {
    m_materials.clear();
    m_stats = Statistics();
    Logger::Info("All materials cleared");
}

size_t MaterialController::GetMaterialCount() const { return m_materials.size(); }

std::vector<std::string> MaterialController::GetMaterialNames() const {
    std::vector<std::string> names; names.reserve(m_materials.size());
    for (const auto& pair : m_materials) names.push_back(pair.first);
    return names;
}

std::vector<Material*> MaterialController::GetMaterialsByShader(const std::string& shaderName) const {
    std::vector<Material*> result;
    Logger::Debug("Retrieving materials with shader: " + shaderName);

    for (const auto& pair : m_materials) {
        Material* material = pair.second.get();
        if (material->GetShader() && material->GetShader()->GetName() == shaderName) {
            result.push_back(material);
        }
    }

    if (result.empty()) {
        Logger::Warning("No materials found with shader: " + shaderName);
    }
    else {
        Logger::Info("Found " + std::to_string(result.size()) + " materials with shader: " + shaderName);
    }

    return result;
}


void MaterialController::PrintDebugInfo() const {
    Logger::Info("=== MaterialController Debug Info ===");
    Logger::Info("Total materials: " + std::to_string(m_stats.totalMaterials));
}

bool MaterialController::SaveMaterialToFile(const std::string& name, const std::string& filepath) {
    auto* material = GetMaterial(name);
    if (!material) { Logger::Error("Material not found: " + name); return false; }
    return JsonMaterialSerializer::SaveToFile(*material, filepath);
}

Material* MaterialController::LoadMaterialFromFile(const std::string& filepath, const std::string& materialName) {
    auto material = JsonMaterialSerializer::LoadFromFile(filepath);
    if (!material) return nullptr;
    std::string name = materialName.empty() ? material->GetName() : materialName;
    material->SetName(name);
    name = GenerateUniqueName(name);
    material->SetName(name);
    Material* ptr = material.get();
    m_materials[name] = std::move(material);
    m_stats.totalMaterials++;
    auto* matPtr = m_materials[name].get();
    if (dynamic_cast<BasicMaterial*>(matPtr)) m_stats.basicMaterials++;
    else if (dynamic_cast<PBRMaterial*>(matPtr)) m_stats.pbrMaterials++;
    else if (dynamic_cast<MaterialInstance*>(matPtr)) m_stats.materialInstances++;
    Logger::Info("Material loaded from file: " + name);
    return ptr;
}

} // namespace ogle
