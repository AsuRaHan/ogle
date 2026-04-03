#include "World.h"

#include "core/FileSystem.h"

#include <cmath>
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

    World::World() = default;

    World::~World() = default;

    WorldObject World::CreateWorldObject(const std::string& name, WorldObjectKind kind) {
        const Entity entity = m_registry.create();
        m_registry.emplace<WorldObjectComponent>(entity, kind, true, true);
        m_registry.emplace<NameComponent>(entity, name);
        m_registry.emplace<TransformComponent>(entity);
        return WorldObject(this, entity);
    }

    WorldObject World::CreateMeshObject(const std::string& name) {
        return CreateWorldObject(name, WorldObjectKind::Mesh);
    }

    WorldObject World::GetWorldObject(Entity entity) {
        return WorldObject(this, entity);
    }

    const WorldObject World::GetWorldObject(Entity entity) const {
        return WorldObject(const_cast<World*>(this), entity);
    }

    Entity World::CreateEntity(const std::string& name) {
        return CreateWorldObject(name, WorldObjectKind::Generic).GetEntity();
    }

    void World::DestroyEntity(Entity entity) {
        if (IsValid(entity)) {
            m_registry.destroy(entity);
        }
    }

    Entity World::FindEntityByName(const std::string& name) const {
        auto view = m_registry.view<NameComponent>();
        for (auto entity : view) {
            const auto& nameComponent = view.get<NameComponent>(entity);
            if (nameComponent.value == name) {
                return entity;
            }
        }

        return entt::null;
    }

    Entity World::AddModel(std::shared_ptr<ModelEntity> model, const std::string& name) {
        const Entity entity = CreateMeshObject(name).GetEntity();
        auto& transform = m_registry.get<TransformComponent>(entity);
        transform.position = model ? model->GetPosition() : glm::vec3(0.0f);
        transform.rotation = model ? model->GetRotation() : glm::vec3(0.0f);
        transform.scale = model ? model->GetScale() : glm::vec3(1.0f);
        m_registry.emplace<ModelComponent>(entity, std::move(model));
        m_registry.emplace<MaterialComponent>(entity, m_registry.get<ModelComponent>(entity).model ? m_registry.get<ModelComponent>(entity).model->GetMaterial() : Material{});
        m_registry.emplace<PrimitiveComponent>(entity);
        SyncModelTransform(entity);
        return entity;
    }

    Entity World::CreateModelFromFile(const std::string& filePath, ModelType type, const std::string& name) {
        auto model = std::make_shared<ModelEntity>(type, filePath);
        if (!model->LoadFromFile(filePath)) {
            return entt::null;
        }
        model->BakeToGPU();
        if (!model->GetLoadedDiffuseTexturePath().empty()) {
            model->SetDiffuseTexturePath(model->GetLoadedDiffuseTexturePath());
        }
        const Entity entity = AddModel(std::move(model), name);
        if (m_registry.all_of<PrimitiveComponent>(entity)) {
            auto& primitive = m_registry.get<PrimitiveComponent>(entity);
            primitive.type = PrimitiveType::ModelFile;
            primitive.sourcePath = filePath;
        }

        const ModelEntity* loadedModel = GetModel(entity);
        if (loadedModel) {
            const auto& modelClips = loadedModel->GetAnimationClips();
            if (!modelClips.empty()) {
                if (!m_registry.all_of<AnimationComponent>(entity)) {
                    m_registry.emplace<AnimationComponent>(entity);
                }
                auto& animation = m_registry.get<AnimationComponent>(entity);
                animation.clips = modelClips;
                animation.currentClipIndex = 0;
                animation.currentClip = modelClips[0].name;
                animation.duration = modelClips[0].duration;
                animation.currentTime = 0.0f;
                animation.enabled = true;
            }

            if (loadedModel->GetBoneCount() > 0) {
                SkeletonComponent skeleton;
                skeleton.enabled = true;
                skeleton.boneCount = loadedModel->GetBoneCount();
                skeleton.sourcePath = filePath;
                m_registry.emplace<SkeletonComponent>(entity, skeleton);
            }
        }

        return entity;
    }

    void World::Update(float deltaTime) {
        auto view = m_registry.view<TransformComponent, ModelComponent>();
        for (auto entity : view) {
            SyncModelTransform(entity);
        }

        auto animationView = m_registry.view<AnimationComponent>();
        for (auto entity : animationView) {
            auto& animation = animationView.get<AnimationComponent>(entity);
            if (!animation.enabled || !animation.playing) {
                continue;
            }

            animation.currentTime += animation.playbackSpeed * deltaTime;

            float maxTime = animation.duration > 0.0f ? animation.duration : 3600.0f;
            if (animation.currentClipIndex >= 0 && animation.currentClipIndex < static_cast<int>(animation.clips.size())) {
                maxTime = animation.clips[animation.currentClipIndex].duration;
            }

            if (animation.loop) {
                if (animation.currentTime >= maxTime) {
                    animation.currentTime = fmod(animation.currentTime, maxTime);
                }
            } else {
                if (animation.currentTime >= maxTime) {
                    animation.currentTime = maxTime;
                    animation.playing = false;
                }
            }

            if (animation.currentTime < 0.0f) {
                animation.currentTime = 0.0f;
            }
        }
    }

    void World::Draw() {
        auto view = m_registry.view<ModelComponent>();
        for (auto entity : view) {
            auto& model = view.get<ModelComponent>(entity);
            if (model.model) {
                model.model->Draw();
            }
        }
    }

    void World::Clear() {
        m_registry.clear();
    }

    void World::Save(const std::string& path) {
        nlohmann::json j;
        auto view = m_registry.view<NameComponent, TransformComponent, WorldObjectComponent>();
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

            if (m_registry.all_of<ModelComponent>(entity)) {
                auto& model = m_registry.get<ModelComponent>(entity);
                if (model.model) {
                    nlohmann::json modelJson;
                    model.model->ToJson(modelJson);
                    entityJson["model"] = modelJson;
                }
            }

            if (m_registry.all_of<PrimitiveComponent>(entity)) {
                const auto& primitive = m_registry.get<PrimitiveComponent>(entity);
                entityJson["primitive"] = {
                    {"type", static_cast<int>(primitive.type)},
                    {"sourcePath", primitive.sourcePath}
                };
            }

            if (m_registry.all_of<MaterialComponent>(entity)) {
                entityJson["materialComponent"] = MaterialToJson(m_registry.get<MaterialComponent>(entity).material);
            }

            if (m_registry.all_of<ShaderComponent>(entity)) {
                const auto& shader = m_registry.get<ShaderComponent>(entity);
                entityJson["shader"] = {
                    {"programName", shader.programName}
                };
            }

            if (m_registry.all_of<LightComponent>(entity)) {
                const auto& light = m_registry.get<LightComponent>(entity);
                entityJson["light"] = {
                    {"type", light.type == LightType::Directional ? "Directional" : "Point"},
                    {"color", { light.color.x, light.color.y, light.color.z }},
                    {"intensity", light.intensity},
                    {"range", light.range},
                    {"castShadows", light.castShadows},
                    {"primary", light.primary}
                };
            }

            if (m_registry.all_of<SkeletonComponent>(entity)) {
                const auto& skeleton = m_registry.get<SkeletonComponent>(entity);
                entityJson["skeleton"] = {
                    {"enabled", skeleton.enabled},
                    {"boneCount", skeleton.boneCount},
                    {"sourcePath", skeleton.sourcePath}
                };
            }

            if (m_registry.all_of<AnimationComponent>(entity)) {
                const auto& animation = m_registry.get<AnimationComponent>(entity);
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

            if (m_registry.all_of<ScriptComponent>(entity)) {
                const auto& script = m_registry.get<ScriptComponent>(entity);
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

    void World::Load(const std::string& path) {
        m_registry.clear();

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
            const Entity entity = CreateWorldObject(name, kind).GetEntity();

            auto& worldObject = m_registry.get<WorldObjectComponent>(entity);
            worldObject.enabled = entityJson.value("enabled", true);
            worldObject.visible = entityJson.value("visible", true);

            auto& transform = m_registry.get<TransformComponent>(entity);
            const auto& positionJson = entityJson.at("position");
            const auto& rotationJson = entityJson.at("rotation");
            const auto& scaleJson = entityJson.at("scale");
            transform.position = glm::vec3(positionJson[0], positionJson[1], positionJson[2]);
            transform.rotation = glm::vec3(rotationJson[0], rotationJson[1], rotationJson[2]);
            transform.scale = glm::vec3(scaleJson[0], scaleJson[1], scaleJson[2]);

            if (entityJson.contains("model")) {
                auto model = std::make_shared<ModelEntity>();
                model->FromJson(entityJson.at("model"));
                m_registry.emplace<ModelComponent>(entity, model);
                m_registry.emplace<MaterialComponent>(entity, model->GetMaterial());
            }

            if (entityJson.contains("primitive")) {
                const auto& primitiveJson = entityJson.at("primitive");
                PrimitiveComponent primitive;
                primitive.type = static_cast<PrimitiveType>(primitiveJson.value("type", static_cast<int>(PrimitiveType::None)));
                primitive.sourcePath = primitiveJson.value("sourcePath", std::string());
                m_registry.emplace<PrimitiveComponent>(entity, primitive);
            }

            if (entityJson.contains("materialComponent")) {
                MaterialComponent materialComponent;
                MaterialFromJson(entityJson.at("materialComponent"), materialComponent.material);
                if (m_registry.all_of<MaterialComponent>(entity)) {
                    m_registry.replace<MaterialComponent>(entity, materialComponent);
                } else {
                    m_registry.emplace<MaterialComponent>(entity, materialComponent);
                }
            }

            if (entityJson.contains("shader")) {
                const auto& shaderJson = entityJson.at("shader");
                ShaderComponent shader;
                shader.programName = shaderJson.value("programName", std::string("default"));
                m_registry.emplace<ShaderComponent>(entity, shader);
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
                m_registry.emplace<LightComponent>(entity, light);
            }

            if (entityJson.contains("skeleton")) {
                const auto& skeletonJson = entityJson.at("skeleton");
                SkeletonComponent skeleton;
                skeleton.enabled = skeletonJson.value("enabled", false);
                skeleton.boneCount = skeletonJson.value("boneCount", 0);
                skeleton.sourcePath = skeletonJson.value("sourcePath", std::string());
                m_registry.emplace<SkeletonComponent>(entity, skeleton);
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

                if (m_registry.all_of<ModelComponent>(entity)) {
                    const ModelEntity* model = m_registry.get<ModelComponent>(entity).model.get();
                    if (model) {
                        animation.clips = model->GetAnimationClips();
                        if (!animation.clips.empty() && animation.currentClip.empty()) {
                            animation.currentClipIndex = 0;
                            animation.currentClip = animation.clips[0].name;
                            animation.duration = animation.clips[0].duration;
                        }
                    }
                }

                m_registry.emplace<AnimationComponent>(entity, animation);
            }

            if (entityJson.contains("script")) {
                const auto& scriptJson = entityJson.at("script");
                ScriptComponent script;
                script.enabled = scriptJson.value("enabled", true);
                script.autoStart = scriptJson.value("autoStart", false);
                script.scriptPath = scriptJson.value("scriptPath", std::string());
                m_registry.emplace<ScriptComponent>(entity, script);
            }

            SyncModelTransform(entity);
        }
    }

    bool World::IsValid(Entity entity) const {
        return m_registry.valid(entity);
    }

    bool World::HasModel(Entity entity) const {
        return IsValid(entity) && m_registry.all_of<ModelComponent>(entity);
    }

    WorldObjectComponent* World::GetWorldObjectComponent(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<WorldObjectComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<WorldObjectComponent>(entity);
    }

    const WorldObjectComponent* World::GetWorldObjectComponent(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<WorldObjectComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<WorldObjectComponent>(entity);
    }

    NameComponent* World::GetNameComponent(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<NameComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<NameComponent>(entity);
    }

    const NameComponent* World::GetNameComponent(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<NameComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<NameComponent>(entity);
    }

    void World::SetName(Entity entity, const std::string& name) {
        NameComponent* nameComponent = GetNameComponent(entity);
        if (nameComponent) {
            nameComponent->value = name;
        }
    }

    ModelEntity* World::GetModel(Entity entity) {
        if (!HasModel(entity)) {
            return nullptr;
        }

        auto& component = m_registry.get<ModelComponent>(entity);
        return component.model.get();
    }

    const ModelEntity* World::GetModel(Entity entity) const {
        if (!HasModel(entity)) {
            return nullptr;
        }

        const auto& component = m_registry.get<ModelComponent>(entity);
        return component.model.get();
    }

    TransformComponent* World::GetTransform(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<TransformComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<TransformComponent>(entity);
    }

    const TransformComponent* World::GetTransform(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<TransformComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<TransformComponent>(entity);
    }

    PrimitiveComponent* World::GetPrimitive(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<PrimitiveComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<PrimitiveComponent>(entity);
    }

    const PrimitiveComponent* World::GetPrimitive(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<PrimitiveComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<PrimitiveComponent>(entity);
    }

    MaterialComponent* World::GetMaterial(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<MaterialComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<MaterialComponent>(entity);
    }

    const MaterialComponent* World::GetMaterial(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<MaterialComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<MaterialComponent>(entity);
    }

    LightComponent* World::GetLight(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<LightComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<LightComponent>(entity);
    }

    ShaderComponent* World::GetShader(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<ShaderComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<ShaderComponent>(entity);
    }

    const ShaderComponent* World::GetShader(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<ShaderComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<ShaderComponent>(entity);
    }

    SkeletonComponent* World::GetSkeleton(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<SkeletonComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<SkeletonComponent>(entity);
    }

    const SkeletonComponent* World::GetSkeleton(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<SkeletonComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<SkeletonComponent>(entity);
    }

    AnimationComponent* World::GetAnimation(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<AnimationComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<AnimationComponent>(entity);
    }

    const AnimationComponent* World::GetAnimation(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<AnimationComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<AnimationComponent>(entity);
    }

    ScriptComponent* World::GetScript(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<ScriptComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<ScriptComponent>(entity);
    }

    const ScriptComponent* World::GetScript(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<ScriptComponent>(entity)) {
            return nullptr;
        }
        return &m_registry.get<ScriptComponent>(entity);
    }

    const LightComponent* World::GetLight(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<LightComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<LightComponent>(entity);
    }

    PhysicsBodyComponent* World::GetPhysicsBody(Entity entity) {
        if (!IsValid(entity) || !m_registry.all_of<PhysicsBodyComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<PhysicsBodyComponent>(entity);
    }

    const PhysicsBodyComponent* World::GetPhysicsBody(Entity entity) const {
        if (!IsValid(entity) || !m_registry.all_of<PhysicsBodyComponent>(entity)) {
            return nullptr;
        }

        return &m_registry.get<PhysicsBodyComponent>(entity);
    }

    void World::SetTransform(
        Entity entity,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale) {

        TransformComponent* transform = GetTransform(entity);
        if (!transform) {
            return;
        }

        transform->position = position;
        transform->rotation = rotation;
        transform->scale = scale;
        SyncModelTransform(entity);
    }

    void World::SyncModelTransform(Entity entity) {
        if (!m_registry.all_of<TransformComponent, ModelComponent>(entity)) {
            return;
        }

        auto& transform = m_registry.get<TransformComponent>(entity);
        auto& model = m_registry.get<ModelComponent>(entity);
        if (!model.model) {
            return;
        }

        model.model->SetPosition(transform.position);
        model.model->SetRotation(transform.rotation);
        model.model->SetScale(transform.scale);
    }

    void World::MakeModelUnique(Entity entity)
    {
        auto* modelComp = m_registry.try_get<ModelComponent>(entity);
        if (!modelComp || !modelComp->model) {
            return;
        }

        if (modelComp->model.use_count() <= 1) {
            return; // Already unique
        }

        // Create a deep copy of the ModelEntity
        auto originalModel = modelComp->model;

        // Create a new model. The file path is made unique to indicate it's a copy.
        auto newModel = std::make_shared<ModelEntity>(originalModel->GetType(), originalModel->GetFilePath() + " (Unique)");

        // Copy geometry data. SetMeshData will create a new MeshBuffer.
        newModel->SetMeshData(originalModel->GetVertices(), originalModel->GetIndices());

        // Copy material
        newModel->GetMaterial() = originalModel->GetMaterial();

        // Replace the shared_ptr in the component
        modelComp->model = newModel;
    }
}
