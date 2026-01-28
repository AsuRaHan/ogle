#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "render/material/Material.h"
#include "render/TextureController.h"
#include "render/ShaderController.h"
#include "render/MaterialController.h"
#include "log/Logger.h"

namespace ogle {

using json = nlohmann::json;

class JsonMaterialSerializer {
public:
    // Сериализация материала в JSON
    static json Serialize(const Material& material);
    static json Serialize(const BasicMaterial& material);
    static json Serialize(const PBRMaterial& material);
    static json Serialize(const MaterialInstance& instance);
    
    // Десериализация из JSON
    static std::unique_ptr<Material> Deserialize(const json& j);
    static std::unique_ptr<BasicMaterial> DeserializeBasic(const json& j);
    static std::unique_ptr<PBRMaterial> DeserializePBR(const json& j);
    static std::unique_ptr<MaterialInstance> DeserializeInstance(const json& j);
    
    // Работа с файлами
    static bool SaveToFile(const Material& material, const std::string& filepath);
    static std::unique_ptr<Material> LoadFromFile(const std::string& filepath);
    
    // Вспомогательные методы
    static json SerializeRenderState(const RenderState& state);
    static RenderState DeserializeRenderState(const json& j);
    
    static json SerializeTextureInfo(const std::string& name, Texture* texture);
    static Texture* DeserializeTextureInfo(const json& j);
    
private:
    // Общие методы сериализации
    static void SerializeCommon(const Material& material, json& j);
    static void DeserializeCommon(Material& material, const json& j);
    
    // Вспомогательные для типов
    static std::string MaterialTypeToString(MaterialType type);
    static MaterialType StringToMaterialType(const std::string& str);
};
}