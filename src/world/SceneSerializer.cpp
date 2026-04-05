#include "SceneSerializer.h"
#include "World.h"
#include "core/FileSystem.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace OGLE {
    namespace {
        nlohmann::json MaterialToJson(const Material& material)
        {
            return {
                {"baseColor", {material.GetBaseColor().x, material.GetBaseColor().y, material.GetBaseColor().z}},
                {"emissiveColor", {material.GetEmissiveColor().x, material.GetEmissiveColor().y, material.GetEmissiveColor().z}},
                {"uvTiling", {material.GetUvTiling().x, material.GetUvTiling().y}},
                {"uvOffset", {material.GetUvOffset().x, material.GetUvOffset().y}},
                {"roughness", material.GetRoughness()},
                {"metallic", material.GetMetallic()},
                {"alphaCutoff", material.GetAlphaCutoff()},
                {"diffuseTexturePath", material.GetDiffuseTexturePath()},
                {"emissiveTexturePath", material.GetEmissiveTexturePath()},
                {"shaderProgram", material.GetShaderProgram()}
            };
        }

        void MaterialFromJson(const nlohmann::json& materialJson, Material& material)
        {
            if (materialJson.contains("baseColor")) {
                const auto& baseColorJson = materialJson.at("baseColor");
                material.SetBaseColor(glm::vec3(baseColorJson[0], baseColorJson[1], baseColorJson[2]));
            }
            if (materialJson.contains("emissiveColor")) {
                const auto& emissiveColorJson = materialJson.at("emissiveColor");
                material.SetEmissiveColor(glm::vec3(emissiveColorJson[0], emissiveColorJson[1], emissiveColorJson[2]));
            }
            if (materialJson.contains("uvTiling")) {
                const auto& uvTilingJson = materialJson.at("uvTiling");
                material.SetUvTiling(glm::vec2(uvTilingJson[0], uvTilingJson[1]));
            }
            if (materialJson.contains("uvOffset")) {
                const auto& uvOffsetJson = materialJson.at("uvOffset");
                material.SetUvOffset(glm::vec2(uvOffsetJson[0], uvOffsetJson[1]));
            }
            if (materialJson.contains("roughness")) {
                material.SetRoughness(materialJson.at("roughness").get<float>());
            }
            if (materialJson.contains("metallic")) {
                material.SetMetallic(materialJson.at("metallic").get<float>());
            }
            if (materialJson.contains("alphaCutoff")) {
                material.SetAlphaCutoff(materialJson.at("alphaCutoff").get<float>());
            }
            if (materialJson.contains("diffuseTexturePath")) {
                material.SetDiffuseTexturePath(materialJson.at("diffuseTexturePath").get<std::string>());
            }
            if (materialJson.contains("emissiveTexturePath")) {
                material.SetEmissiveTexturePath(materialJson.at("emissiveTexturePath").get<std::string>());
            }
            if (materialJson.contains("shaderProgram")) {
                material.SetShaderProgram(materialJson.at("shaderProgram").get<std::string>());
            }
        }
    }

    SceneSerializer::SceneSerializer(World& world) : m_world(world) {}

    void SceneSerializer::Save(const std::string& path) {
        nlohmann::json j;
        auto& registry = m_world.GetRegistry();
        auto view = registry.view<NameComponent, TransformComponent, WorldObjectComponent>();
        for (auto entity : view) {
            auto& name = view.get<NameComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);
            auto& worldObject = view.get<WorldObjectComponent>(entity);

            nlohmann::json entityJson;
            entityJson["name"] = name.value;
            entityJson["kind"] = static_cast<int>(worldObject.kind);
            entityJson["enabled"] = worldObject.enabled;
            entityJson["visible"] = worldObject.visible;
            entityJson["position"] = { transform.position.x, transform.position.y, transform.position.z };
            entityJson["rotation"] = { transform.rotation.x, transform.rotation.y, transform.rotation.z };
            entityJson["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };

            if (registry.all_of<ModelComponent>(entity)) {
                auto& model = registry.get<ModelComponent>(entity);
                if (model.model) {
                    nlohmann::json modelJson;
                    model.model->ToJson(modelJson);
                    entityJson["model"] = modelJson;
                }
            }

            if (registry.all_of<PrimitiveComponent>(entity)) {
                const auto& primitive = registry.get<PrimitiveComponent>(entity);
                entityJson["primitive"] = {
                    {"type", static_cast<int>(primitive.type)},
                    {"sourcePath", primitive.sourcePath}
                };
            }

            if (registry.all_of<MaterialComponent>(entity)) {
                entityJson["materialComponent"] = MaterialToJson(registry.get<MaterialComponent>(entity).material);
            }

            if (registry.all_of<ShaderComponent>(entity)) {
                const auto& shader = registry.get<ShaderComponent>(entity);
                entityJson["shader"] = {
                    {"programName", shader.programName}
                };
            }

            if (registry.all_of<LightComponent>(entity)) {
                const auto& light = registry.get<LightComponent>(entity);
                entityJson["light"] = {
                    {"type", light.type == LightType::Directional ? "Directional" : "Point"},
                    {"color", { light.color.x, light.color.y, light.color.z }},
                    {"intensity", light.intensity},
                    {"range", light.range},
                    {"castShadows", light.castShadows},
                    {"primary", light.primary}
                };
            }

            if (registry.all_of<SkeletonComponent>(entity)) {
                const auto& skeleton = registry.get<SkeletonComponent>(entity);
                entityJson["skeleton"] = {
                    {"enabled", skeleton.enabled},
                    {"boneCount", skeleton.boneCount},
                    {"sourcePath", skeleton.sourcePath}
                };
            }

            if (registry.all_of<AnimationComponent>(entity)) {
                const auto& animation = registry.get<AnimationComponent>(entity);
                entityJson["animation"] = {
                    {"enabled", animation.enabled},
                    {"playing", animation.playing},
                    {"loop", animation.loop},
                    {"currentTime", animation.currentTime},
                    {"playbackSpeed", animation.playbackSpeed},
                    {"duration", animation.duration},
                    {"currentClip", animation.currentClip}
                };
            }

            if (registry.all_of<ScriptComponent>(entity)) {
                const auto& script = registry.get<ScriptComponent>(entity);
                entityJson["script"] = {
                    {"enabled", script.enabled},
                    {"autoStart", script.autoStart},
                    {"scriptPath", script.scriptPath}
                };
            }

            j["entities"].push_back(entityJson);
        }

        FileSystem::EnsureParentDirectory(path);
        std::ofstream file(path);
        file << j.dump(4);
    }

    void SceneSerializer::Load(const std::string& path) {
        m_world.Clear();
        auto& registry = m_world.GetRegistry();

        const std::filesystem::path resolvedPath = FileSystem::ResolvePath(path);
        std::ifstream file(resolvedPath);
        if (!file.is_open()) {
            return;
        }

        nlohmann::json j;
        file >> j;

        if (!j.contains("entities")) {
            return;
        }

        for (const auto& entityJson : j["entities"]) {
            const std::string name = entityJson.value("name", "Model");
            const WorldObjectKind kind = static_cast<WorldObjectKind>(entityJson.value("kind", static_cast<int>(WorldObjectKind::Generic)));
            const Entity entity = m_world.CreateWorldObject(name, kind).GetEntity();

            auto& worldObject = registry.get<WorldObjectComponent>(entity);
            worldObject.enabled = entityJson.value("enabled", true);
            worldObject.visible = entityJson.value("visible", true);

            auto& transform = registry.get<TransformComponent>(entity);
            const auto& positionJson = entityJson.at("position");
            const auto& rotationJson = entityJson.at("rotation");
            const auto& scaleJson = entityJson.at("scale");
            transform.position = glm::vec3(positionJson[0], positionJson[1], positionJson[2]);
            transform.rotation = glm::vec3(rotationJson[0], rotationJson[1], rotationJson[2]);
            transform.scale = glm::vec3(scaleJson[0], scaleJson[1], scaleJson[2]);

            if (entityJson.contains("model")) {
                auto model = std::make_shared<ModelEntity>();
                model->FromJson(entityJson.at("model"));
                registry.emplace<ModelComponent>(entity, model);
                registry.emplace<MaterialComponent>(entity, model->GetMaterial());
            }

            if (entityJson.contains("primitive")) {
                const auto& primitiveJson = entityJson.at("primitive");
                PrimitiveComponent primitive;
                primitive.type = static_cast<PrimitiveType>(primitiveJson.value("type", static_cast<int>(PrimitiveType::None)));
                primitive.sourcePath = primitiveJson.value("sourcePath", std::string());
                registry.emplace<PrimitiveComponent>(entity, primitive);
            }

            if (entityJson.contains("materialComponent")) {
                MaterialComponent materialComponent;
                MaterialFromJson(entityJson.at("materialComponent"), materialComponent.material);
                registry.emplace_or_replace<MaterialComponent>(entity, materialComponent);
            }

            if (entityJson.contains("shader")) {
                const auto& shaderJson = entityJson.at("shader");
                ShaderComponent shader;
                shader.programName = shaderJson.value("programName", std::string("default"));
                registry.emplace<ShaderComponent>(entity, shader);
            }

            if (entityJson.contains("light")) {
                const auto& lightJson = entityJson.at("light");
                LightComponent light;
                light.type = lightJson.value("type", std::string("Directional")) == "Point"
                    ? LightType::Point
                    : LightType::Directional;
                if (lightJson.contains("color")) {
                    const auto& colorJson = lightJson.at("color");
                    light.color = glm::vec3(colorJson[0], colorJson[1], colorJson[2]);
                }
                light.intensity = lightJson.value("intensity", 1.0f);
                light.range = lightJson.value("range", 10.0f);
                light.castShadows = lightJson.value("castShadows", false);
                light.primary = lightJson.value("primary", false);
                registry.emplace<LightComponent>(entity, light);
            }

            if (entityJson.contains("skeleton")) {
                const auto& skeletonJson = entityJson.at("skeleton");
                SkeletonComponent skeleton;
                skeleton.enabled = skeletonJson.value("enabled", false);
                skeleton.boneCount = skeletonJson.value("boneCount", 0);
                skeleton.sourcePath = skeletonJson.value("sourcePath", std::string());
                registry.emplace<SkeletonComponent>(entity, skeleton);
            }

            if (entityJson.contains("animation")) {
                const auto& animationJson = entityJson.at("animation");
                AnimationComponent animation;
                animation.enabled = animationJson.value("enabled", false);
                animation.playing = animationJson.value("playing", false);
                animation.loop = animationJson.value("loop", true);
                animation.currentTime = animationJson.value("currentTime", 0.0f);
                animation.playbackSpeed = animationJson.value("playbackSpeed", 1.0f);
                animation.duration = animationJson.value("duration", 1.0f);
                animation.currentClip = animationJson.value("currentClip", std::string());

                if (registry.all_of<ModelComponent>(entity)) {
                    const ModelEntity* model = registry.get<ModelComponent>(entity).model.get();
                    if (model) {
                        animation.clips = model->GetAnimationClips();
                        if (!animation.clips.empty() && animation.currentClip.empty()) {
                            animation.currentClipIndex = 0;
                            animation.currentClip = animation.clips[0].name;
                            animation.duration = animation.clips[0].duration;
                        }
                    }
                }

                registry.emplace<AnimationComponent>(entity, animation);
            }

            if (entityJson.contains("script")) {
                const auto& scriptJson = entityJson.at("script");
                ScriptComponent script;
                script.enabled = scriptJson.value("enabled", true);
                script.autoStart = scriptJson.value("autoStart", false);
                script.scriptPath = scriptJson.value("scriptPath", std::string());
                registry.emplace<ScriptComponent>(entity, script);
            }

            m_world.SyncModelTransform(entity);
        }
    }
}