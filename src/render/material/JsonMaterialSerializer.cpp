#include "JsonMaterialSerializer.h"

namespace fs = std::filesystem;

namespace ogle {

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================

std::string JsonMaterialSerializer::MaterialTypeToString(MaterialType type) {
    switch (type) {
        case MaterialType::Basic: return "Basic";
        case MaterialType::PBR: return "PBR";
        case MaterialType::Unlit: return "Unlit";
        case MaterialType::Custom: return "Custom";
        default: return "Unknown";
    }
}

MaterialType JsonMaterialSerializer::StringToMaterialType(const std::string& str) {
    if (str == "Basic") return MaterialType::Basic;
    if (str == "PBR") return MaterialType::PBR;
    if (str == "Unlit") return MaterialType::Unlit;
    if (str == "Custom") return MaterialType::Custom;
    return MaterialType::Basic;
}

// ==================== СЕРИАЛИЗАЦИЯ RENDER STATE ====================

json JsonMaterialSerializer::SerializeRenderState(const RenderState& state) {
    json j;
    
    j["depthTest"] = state.depthTest;
    j["depthWrite"] = state.depthWrite;
    j["depthFunc"] = state.depthFunc;
    j["cullFace"] = state.cullFace;
    j["cullMode"] = state.cullMode;
    j["blend"] = state.blend;
    j["blendSrc"] = state.blendSrc;
    j["blendDst"] = state.blendDst;
    j["wireframe"] = state.wireframe;
    j["lineWidth"] = state.lineWidth;
    
    return j;
}

RenderState JsonMaterialSerializer::DeserializeRenderState(const json& j) {
    RenderState state;
    
    if (j.contains("depthTest")) state.depthTest = j["depthTest"];
    if (j.contains("depthWrite")) state.depthWrite = j["depthWrite"];
    if (j.contains("depthFunc")) state.depthFunc = j["depthFunc"];
    if (j.contains("cullFace")) state.cullFace = j["cullFace"];
    if (j.contains("cullMode")) state.cullMode = j["cullMode"];
    if (j.contains("blend")) state.blend = j["blend"];
    if (j.contains("blendSrc")) state.blendSrc = j["blendSrc"];
    if (j.contains("blendDst")) state.blendDst = j["blendDst"];
    if (j.contains("wireframe")) state.wireframe = j["wireframe"];
    if (j.contains("lineWidth")) state.lineWidth = j["lineWidth"];
    
    return state;
}

// ==================== СЕРИАЛИЗАЦИЯ ТЕКСТУР ====================

json JsonMaterialSerializer::SerializeTextureInfo(const std::string& name, Texture* texture) {
    json j;
    
    if (!texture) {
        j["name"] = name;
        j["loaded"] = false;
        return j;
    }
    
    j["name"] = name;
    j["textureName"] = texture->GetName();
    j["type"] = static_cast<int>(texture->GetType());
    j["width"] = texture->GetWidth();
    j["height"] = texture->GetHeight();
    j["loaded"] = true;
    
    return j;
}

Texture* JsonMaterialSerializer::DeserializeTextureInfo(const json& j) {
    if (!j.contains("loaded") || !j["loaded"]) {
        return nullptr;
    }
    
    if (j.contains("textureName")) {
        std::string textureName = j["textureName"];
        auto& texCtrl = TextureController::Get();
        return texCtrl.GetTexture(textureName);
    }
    
    return nullptr;
}

// ==================== ОБЩИЕ МЕТОДЫ СЕРИАЛИЗАЦИИ ====================

void JsonMaterialSerializer::SerializeCommon(const Material& material, json& j) {
    j["name"] = material.GetName();
    j["type"] = MaterialTypeToString(material.GetType());
    j["renderState"] = SerializeRenderState(material.GetRenderState());

    // Шейдер
    auto shader = material.GetShader();
    if (shader) {
        j["shader"] = shader->GetName();
    }

    // Float параметры
    auto floatNames = material.GetFloatParamNames();
    for (const auto& name : floatNames) {
        j["floatParams"][name] = material.GetFloat(name);
    }

    // Int параметры
    auto intNames = material.GetIntParamNames();
    for (const auto& name : intNames) {
        j["intParams"][name] = material.GetInt(name);
    }

    // Vec3 параметры
    auto vec3Names = material.GetVec3ParamNames();
    for (const auto& name : vec3Names) {
        glm::vec3 value = material.GetVec3(name);
        j["vec3Params"][name] = { value.x, value.y, value.z };
    }

    // Vec4 параметры
    auto vec4Names = material.GetVec4ParamNames();
    for (const auto& name : vec4Names) {
        glm::vec4 value = material.GetVec4(name);
        j["vec4Params"][name] = { value.x, value.y, value.z, value.w };
    }

    // Текстуры (теперь включает ВСЕ текстуры, включая PBR-specific)
    auto textureNames = material.GetTextureParamNames();
    for (const auto& name : textureNames) {
        Texture* texture = material.GetTexture(name);
        j["textureParams"][name] = SerializeTextureInfo(name, texture);
    }
}

void JsonMaterialSerializer::DeserializeCommon(Material& material, const json& j) {
    if (j.contains("name")) {
        material.SetName(j["name"]);
    }
    
    if (j.contains("renderState")) {
        material.SetRenderState(DeserializeRenderState(j["renderState"]));
    }
    
    // Шейдер
    if (j.contains("shader")) {
        auto& shaderCtrl = ShaderController::Get();
        auto shader = shaderCtrl.GetProgram(j["shader"]);
        if (shader) {
            material.SetShader(shader.get());
        }
    }
    
    // Float параметры
    if (j.contains("floatParams")) {
        for (auto& [key, value] : j["floatParams"].items()) {
            material.SetFloat(key, value.get<float>());
        }
    }
    
    // Int параметры
    if (j.contains("intParams")) {
        for (auto& [key, value] : j["intParams"].items()) {
            material.SetInt(key, value.get<int>());
        }
    }
    
    // Vec3 параметры
    if (j.contains("vec3Params")) {
        for (auto& [key, value] : j["vec3Params"].items()) {
            glm::vec3 v(value[0], value[1], value[2]);
            material.SetVec3(key, v);
        }
    }
    
    // Vec4 параметры
    if (j.contains("vec4Params")) {
        for (auto& [key, value] : j["vec4Params"].items()) {
            glm::vec4 v(value[0], value[1], value[2], value[3]);
            material.SetVec4(key, v);
        }
    }
    
    // Текстуры (десериализуем только ссылки)
    if (j.contains("textureParams")) {
        for (auto& [key, value] : j["textureParams"].items()) {
            Texture* texture = DeserializeTextureInfo(value);
            if (texture) {
                material.SetTexture(key, texture);
            }
        }
    }
}

// ==================== СЕРИАЛИЗАЦИЯ КОНКРЕТНЫХ МАТЕРИАЛОВ ====================

json JsonMaterialSerializer::Serialize(const Material& material) {
    // Вызываем соответствующую специализированную версию
    switch (material.GetType()) {
    case MaterialType::Basic: {
        if (auto* basic = dynamic_cast<const BasicMaterial*>(&material)) {
            return Serialize(*basic);
        }
        break;
    }
    case MaterialType::PBR: {
        if (auto* pbr = dynamic_cast<const PBRMaterial*>(&material)) {
            return Serialize(*pbr);
        }
        break;
    }
    case MaterialType::Custom: {
        // Для кастомного материала используем общую сериализацию
        json j;
        SerializeCommon(material, j);
        j["type"] = "Custom";
        return j;
    }
    default: {
        json j;
        j["name"] = material.GetName();
        j["type"] = "Unknown";
        j["error"] = "Unsupported material type";
        return j;
    }
    }

    // Fallback
    json j;
    SerializeCommon(material, j);
    j["type"] = "Unknown";
    return j;
}

json JsonMaterialSerializer::Serialize(const PBRMaterial& material) {
    json j;
    SerializeCommon(material, j);
    j["type"] = "PBR";

    // PBR свойства (дублируем для удобства чтения JSON)
    j["albedo"] = { material.GetAlbedo().x, material.GetAlbedo().y,
                   material.GetAlbedo().z, material.GetAlbedo().w };
    j["metallic"] = material.GetMetallic();
    j["roughness"] = material.GetRoughness();
    j["ao"] = material.GetAO();
    j["emission"] = { material.GetEmission().x, material.GetEmission().y,
                     material.GetEmission().z };

    return j;
}

json JsonMaterialSerializer::Serialize(const BasicMaterial& material) {
    json j;
    SerializeCommon(material, j);
    j["type"] = "Basic";
    
    // Добавляем специфичные для Basic свойства
    j["color"] = {material.GetColor().x, material.GetColor().y, 
                  material.GetColor().z, material.GetColor().w};
    j["useLighting"] = material.GetUseLighting();
    
    return j;
}

json JsonMaterialSerializer::Serialize(const MaterialInstance& instance) {
    json j;
    SerializeCommon(instance, j);
    j["type"] = "Instance";
    
    // Базовый материал
    if (auto base = instance.GetBaseMaterial()) {
        j["baseMaterial"] = base->GetName();
    }
    
    // Пометка что это инстанс
    j["isInstance"] = true;
    
    return j;
}

// ==================== ДЕСЕРИАЛИЗАЦИЯ ====================

std::unique_ptr<Material> JsonMaterialSerializer::Deserialize(const json& j) {
    if (!j.contains("type")) {
        Logger::Error("JSON doesn't contain material type");
        return nullptr;
    }
    
    std::string type = j["type"];
    
    if (type == "Basic") {
        return DeserializeBasic(j);
    } else if (type == "PBR") {
        return DeserializePBR(j);
    } else if (type == "Instance") {
        return DeserializeInstance(j);
    } else {
        // Custom material
        auto material = std::make_unique<BasicMaterial>();
        DeserializeCommon(*material, j);
        return material;
    }
}

std::unique_ptr<BasicMaterial> JsonMaterialSerializer::DeserializeBasic(const json& j) {
    auto material = std::make_unique<BasicMaterial>();
    DeserializeCommon(*material, j);
    
    // Загружаем специфичные свойства
    if (j.contains("color")) {
        glm::vec4 color(j["color"][0], j["color"][1], 
                       j["color"][2], j["color"][3]);
        material->SetColor(color);
    }
    
    if (j.contains("useLighting")) {
        material->SetUseLighting(j["useLighting"]);
    }
    
    return material;
}

std::unique_ptr<PBRMaterial> JsonMaterialSerializer::DeserializePBR(const json& j) {
    auto material = std::make_unique<PBRMaterial>();
    DeserializeCommon(*material, j);
    
    // PBR свойства
    if (j.contains("albedo")) {
        glm::vec4 albedo(j["albedo"][0], j["albedo"][1], 
                        j["albedo"][2], j["albedo"][3]);
        material->SetAlbedo(albedo);
    }
    
    if (j.contains("metallic")) material->SetMetallic(j["metallic"]);
    if (j.contains("roughness")) material->SetRoughness(j["roughness"]);
    if (j.contains("ao")) material->SetAO(j["ao"]);
    
    if (j.contains("emission")) {
        glm::vec3 emission(j["emission"][0], j["emission"][1], j["emission"][2]);
        material->SetEmission(emission);
    }
    
    // Специфичные текстуры PBR
    auto& texCtrl = TextureController::Get();
    
    if (j.contains("albedoMap")) {
        auto tex = texCtrl.GetTexture(j["albedoMap"]);
        material->SetAlbedoMap(tex);
    }
    
    if (j.contains("normalMap")) {
        auto tex = texCtrl.GetTexture(j["normalMap"]);
        material->SetNormalMap(tex);
    }
    
    if (j.contains("metallicRoughnessMap")) {
        auto tex = texCtrl.GetTexture(j["metallicRoughnessMap"]);
        material->SetMetallicRoughnessMap(tex);
    }
    
    if (j.contains("aoMap")) {
        auto tex = texCtrl.GetTexture(j["aoMap"]);
        material->SetAOMap(tex);
    }
    
    if (j.contains("emissionMap")) {
        auto tex = texCtrl.GetTexture(j["emissionMap"]);
        material->SetEmissionMap(tex);
    }
    
    return material;
}

std::unique_ptr<MaterialInstance> JsonMaterialSerializer::DeserializeInstance(const json& j) {
    // Нужно сначала загрузить базовый материал
    Material* baseMaterial = nullptr;
    
    if (j.contains("baseMaterial")) {
        // В реальности нужно загрузить из MaterialController
        auto& matCtrl = MaterialController::Get();
        baseMaterial = matCtrl.GetMaterial(j["baseMaterial"]);
        
        if (!baseMaterial) {
            Logger::Warning("Base material not found: " + j["baseMaterial"].get<std::string>());
        }
    }
    
    auto instance = std::make_unique<MaterialInstance>(baseMaterial);
    DeserializeCommon(*instance, j);
    
    return instance;
}

// ==================== РАБОТА С ФАЙЛАМИ ====================

bool JsonMaterialSerializer::SaveToFile(const Material& material, const std::string& filepath) {
    try {
        json j = Serialize(material);
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("Failed to open file for writing: " + filepath);
            return false;
        }
        
        file << j.dump(2); // Pretty print with 2 spaces indentation
        file.close();
        
        Logger::Info("Material saved to: " + filepath);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Failed to save material: " + std::string(e.what()));
        return false;
    }
}

std::unique_ptr<Material> JsonMaterialSerializer::LoadFromFile(const std::string& filepath) {
    try {
        if (!fs::exists(filepath)) {
            Logger::Error("Material file not found: " + filepath);
            return nullptr;
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("Failed to open file: " + filepath);
            return nullptr;
        }
        
        json j;
        file >> j;
        file.close();
        
        auto material = Deserialize(j);
        if (material) {
            Logger::Info("Material loaded from: " + filepath);
        }
        
        return material;
    } catch (const std::exception& e) {
        Logger::Error("Failed to load material: " + std::string(e.what()));
        return nullptr;
    }
}

}