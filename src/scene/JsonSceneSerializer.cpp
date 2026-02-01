// src/scene/JsonSceneSerializer.cpp
#include "JsonSceneSerializer.h"
#include "Scene.h"
#include "Components.h"
#include "Mesh.h"
#include "log/Logger.h"
#include "render/MaterialController.h"
#include "render/material/Material.h"
#include <fstream>
#include <filesystem>

namespace ogle {

namespace {
void NoopMaterialDeleter(Material*) {}
}

static void vec3_to_json(const glm::vec3& v, json& j) {
    j = { v.x, v.y, v.z };
}
static glm::vec3 vec3_from_json(const json& j) {
    return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>() };
}
static void quat_to_json(const glm::quat& q, json& j) {
    j = { q.x, q.y, q.z, q.w };
}
static glm::quat quat_from_json(const json& j) {
    return { j[3].get<float>(), j[0].get<float>(), j[1].get<float>(), j[2].get<float>() };
}

bool JsonSceneSerializer::SaveToFile(const Scene& scene, const std::string& filepath) {
    json j = Serialize(scene);
    std::ofstream f(filepath);
    if (!f.is_open()) {
        Logger::Error("JsonSceneSerializer: cannot open file for write: " + filepath);
        return false;
    }
    f << j.dump(2);
    Logger::Info("Scene saved: " + filepath);
    return true;
}

bool JsonSceneSerializer::LoadFromFile(Scene& scene, const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) {
        Logger::Error("JsonSceneSerializer: cannot open file: " + filepath);
        return false;
    }
    json j;
    try {
        f >> j;
    } catch (const std::exception& e) {
        Logger::Error("JsonSceneSerializer: JSON parse error: " + std::string(e.what()));
        return false;
    }
    bool ok = Deserialize(scene, j);
    if (ok) Logger::Info("Scene loaded: " + filepath);
    return ok;
}

json JsonSceneSerializer::Serialize(const Scene& scene) {
    json j;
    j["version"] = kFormatVersion;
    j["entities"] = json::array();

    std::unordered_map<entt::entity, int> entityToIndex;
    int index = 0;
    for (auto e : scene.registry.view<Transform>()) {
        entityToIndex[e] = index++;
    }

    for (auto e : scene.registry.view<Transform>()) {
        j["entities"].push_back(SerializeEntity(scene, e, entityToIndex));
    }

    return j;
}

json JsonSceneSerializer::SerializeEntity(const Scene& scene, entt::entity e, const std::unordered_map<entt::entity, int>& entityToIndex) {
    json j;
    j["index"] = entityToIndex.at(e);

    if (auto* tag = scene.registry.try_get<Tag>(e)) {
        j["name"] = tag->name;
    }

    if (auto* tr = scene.registry.try_get<Transform>(e)) {
        json t;
        vec3_to_json(tr->position, t["position"]);
        quat_to_json(tr->rotation, t["rotation"]);
        vec3_to_json(tr->scale, t["scale"]);
        j["transform"] = t;
    }

    if (auto* bounds = scene.registry.try_get<Bounds>(e)) {
        json b;
        vec3_to_json(bounds->center, b["center"]);
        b["radius"] = bounds->radius;
        j["bounds"] = b;
    }

    int parentIndex = -1;
    if (auto* hier = scene.registry.try_get<Hierarchy>(e)) {
        if (scene.registry.valid(hier->parent)) {
            auto it = entityToIndex.find(hier->parent);
            if (it != entityToIndex.end()) parentIndex = it->second;
        }
    }
    j["parentIndex"] = parentIndex;

    if (scene.registry.all_of<DirectionalLight>(e)) {
        auto& light = scene.registry.get<DirectionalLight>(e);
        json l;
        vec3_to_json(light.color, l["color"]);
        l["intensity"] = light.intensity;
        l["useTransformDirection"] = light.useTransformDirection;
        vec3_to_json(light.direction, l["direction"]);
        j["directionalLight"] = l;
    }

    if (scene.registry.all_of<PointLight>(e)) {
        auto& light = scene.registry.get<PointLight>(e);
        json l;
        vec3_to_json(light.color, l["color"]);
        l["intensity"] = light.intensity;
        l["constant"] = light.constant;
        l["linear"] = light.linear;
        l["quadratic"] = light.quadratic;
        j["pointLight"] = l;
    }

    if (scene.registry.all_of<SpotLight>(e)) {
        auto& light = scene.registry.get<SpotLight>(e);
        json l;
        vec3_to_json(light.color, l["color"]);
        l["intensity"] = light.intensity;
        l["innerAngle"] = light.innerAngle;
        l["outerAngle"] = light.outerAngle;
        l["constant"] = light.constant;
        l["linear"] = light.linear;
        l["quadratic"] = light.quadratic;
        j["spotLight"] = l;
    }

    if (scene.registry.all_of<Mesh>(e)) {
        auto& mesh = scene.registry.get<Mesh>(e);
        json meshJson;
        meshJson["preset"] = "Cube";
        if (mesh.GetMaterial()) {
            meshJson["materialName"] = mesh.GetMaterial()->GetName();
        } else {
            meshJson["materialName"] = "";
        }
        j["mesh"] = meshJson;
    }

    return j;
}

bool JsonSceneSerializer::Deserialize(Scene& scene, const json& j) {
    if (!j.contains("entities") || !j["entities"].is_array()) {
        Logger::Error("JsonSceneSerializer: invalid format, missing entities array");
        return false;
    }

    scene.Clear();
    const json& entities = j["entities"];
    std::vector<entt::entity> indexToEntity;
    indexToEntity.reserve(entities.size());

    for (size_t i = 0; i < entities.size(); ++i) {
        entt::entity e = scene.registry.create();
        scene.registry.emplace<Transform>(e);
        scene.registry.emplace<Bounds>(e);
        indexToEntity.push_back(e);
    }

    for (size_t i = 0; i < entities.size(); ++i) {
        DeserializeEntity(scene, indexToEntity[i], entities[i], static_cast<int>(i), indexToEntity);
    }

    for (size_t i = 0; i < entities.size(); ++i) {
        const json& ent = entities[i];
        int parentIndex = ent.value("parentIndex", -1);
        if (parentIndex >= 0 && parentIndex < static_cast<int>(indexToEntity.size())) {
            scene.SetParent(indexToEntity[i], indexToEntity[static_cast<size_t>(parentIndex)]);
        }
    }

    return true;
}

void JsonSceneSerializer::DeserializeEntity(Scene& scene, entt::entity e, const json& j, int index, const std::vector<entt::entity>& indexToEntity) {
    (void)indexToEntity;

    if (j.contains("name")) {
        scene.registry.emplace_or_replace<Tag>(e, j["name"].get<std::string>());
    }

    if (j.contains("transform")) {
        auto& tr = scene.registry.get<Transform>(e);
        const auto& t = j["transform"];
        if (t.contains("position")) tr.position = vec3_from_json(t["position"]);
        if (t.contains("rotation")) tr.rotation = quat_from_json(t["rotation"]);
        if (t.contains("scale")) tr.scale = vec3_from_json(t["scale"]);
        tr.MarkDirty();
    }

    if (j.contains("bounds")) {
        auto& bounds = scene.registry.get<Bounds>(e);
        const auto& b = j["bounds"];
        if (b.contains("center")) bounds.center = vec3_from_json(b["center"]);
        if (b.contains("radius")) bounds.radius = b.value("radius", 1.0f);
    }

    if (j.contains("directionalLight")) {
        auto& light = scene.registry.emplace<DirectionalLight>(e);
        const auto& l = j["directionalLight"];
        if (l.contains("color")) light.color = vec3_from_json(l["color"]);
        light.intensity = l.value("intensity", 1.0f);
        light.useTransformDirection = l.value("useTransformDirection", true);
        if (l.contains("direction")) light.direction = vec3_from_json(l["direction"]);
    }

    if (j.contains("pointLight")) {
        auto& light = scene.registry.emplace<PointLight>(e);
        const auto& l = j["pointLight"];
        if (l.contains("color")) light.color = vec3_from_json(l["color"]);
        light.intensity = l.value("intensity", 1.0f);
        light.constant = l.value("constant", 1.0f);
        light.linear = l.value("linear", 0.09f);
        light.quadratic = l.value("quadratic", 0.032f);
    }

    if (j.contains("spotLight")) {
        auto& light = scene.registry.emplace<SpotLight>(e);
        const auto& l = j["spotLight"];
        if (l.contains("color")) light.color = vec3_from_json(l["color"]);
        light.intensity = l.value("intensity", 1.0f);
        light.innerAngle = l.value("innerAngle", 0.218f);
        light.outerAngle = l.value("outerAngle", 0.305f);
        light.constant = l.value("constant", 1.0f);
        light.linear = l.value("linear", 0.09f);
        light.quadratic = l.value("quadratic", 0.032f);
    }

    if (j.contains("mesh")) {
        const auto& m = j["mesh"];
        std::string preset = m.value("preset", "Cube");
        std::string materialName = m.value("materialName", "");
        auto& mesh = scene.registry.emplace<Mesh>(e);
        mesh.Initialize();
        if (preset == "Cube") {
            mesh.CreateCube();
        }
        if (!materialName.empty()) {
            Material* mat = MaterialController::Get().GetMaterial(materialName);
            if (mat) mesh.SetMaterial(std::shared_ptr<Material>(mat, &NoopMaterialDeleter));
        }
    }
}

} // namespace ogle
