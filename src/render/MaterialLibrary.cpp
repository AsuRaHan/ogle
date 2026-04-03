#include "render/MaterialLibrary.h"
#include "core/FileSystem.h"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace OGLE {
    MaterialLibrary& MaterialLibrary::Instance() {
        static MaterialLibrary instance;
        return instance;
    }

    bool MaterialLibrary::AddMaterial(const std::string& name, const Material& material) {
        if (name.empty()) {
            return false;
        }

        m_materials[name] = material;
        return true;
    }

    bool MaterialLibrary::RemoveMaterial(const std::string& name) {
        return m_materials.erase(name) > 0;
    }

    Material* MaterialLibrary::GetMaterial(const std::string& name) {
        auto it = m_materials.find(name);
        if (it == m_materials.end()) {
            return nullptr;
        }
        return &it->second;
    }

    const Material* MaterialLibrary::GetMaterial(const std::string& name) const {
        auto it = m_materials.find(name);
        if (it == m_materials.end()) {
            return nullptr;
        }
        return &it->second;
    }

    std::vector<std::string> MaterialLibrary::GetMaterialNames() const {
        std::vector<std::string> names;
        names.reserve(m_materials.size());
        for (const auto& kv : m_materials) {
            names.push_back(kv.first);
        }
        return names;
    }

    bool MaterialLibrary::SaveToFile(const std::string& path) const {
        nlohmann::json root;
        for (const auto& kv : m_materials) {
            root[kv.first] = kv.second.ToJson();
        }

        std::string content = root.dump(4);
        std::filesystem::path filePath(path);

        if (!FileSystem::EnsureParentDirectory(filePath)) {
            return false;
        }

        return FileSystem::WriteTextFile(filePath, content);
    }

    bool MaterialLibrary::LoadFromFile(const std::string& path) {
        std::filesystem::path filePath(path);
        std::string content;

        if (!FileSystem::Exists(filePath)) {
            return false;
        }

        if (!FileSystem::ReadTextFile(filePath, content)) {
            return false;
        }

        nlohmann::json root;
        try {
            root = nlohmann::json::parse(content);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse material library JSON: " + std::string(e.what()));
            return false;
        }

        if (!root.is_object()) {
            return false;
        }

        m_materials.clear();
        for (auto it = root.begin(); it != root.end(); ++it) {
            const std::string name = it.key();
            const nlohmann::json& matJson = it.value();

            Material material;
            if (!material.FromJson(matJson)) {
                LOG_WARN("Failed to deserialize material: " + name);
                continue;
            }

            m_materials[name] = material;
        }

        return true;
    }
}
