#include "render/MaterialLibrary.h"

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
}
