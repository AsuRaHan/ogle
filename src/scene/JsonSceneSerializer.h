// src/scene/JsonSceneSerializer.h
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>

namespace ogle {

class Scene;

using json = nlohmann::json;

class JsonSceneSerializer {
public:
    static const int kFormatVersion = 1;

    static bool SaveToFile(const Scene& scene, const std::string& filepath);
    static bool LoadFromFile(Scene& scene, const std::string& filepath);

    static json Serialize(const Scene& scene);
    static bool Deserialize(Scene& scene, const json& j);

private:
    static json SerializeEntity(const Scene& scene, entt::entity e,
        const std::unordered_map<entt::entity, int>& entityToIndex);
    static void DeserializeEntity(Scene& scene, entt::entity e, const json& j, int index,
        const std::vector<entt::entity>& indexToEntity);
};

} // namespace ogle
