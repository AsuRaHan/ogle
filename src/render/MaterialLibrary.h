#pragma once

#include "Material.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace OGLE {
    class MaterialLibrary {
    public:
        static MaterialLibrary& Instance();

        bool AddMaterial(const std::string& name, const Material& material);
        bool RemoveMaterial(const std::string& name);
        Material* GetMaterial(const std::string& name);
        const Material* GetMaterial(const std::string& name) const;
        std::vector<std::string> GetMaterialNames() const;

        bool SaveToFile(const std::string& path) const;
        bool LoadFromFile(const std::string& path);

    private:
        MaterialLibrary() = default;
        std::unordered_map<std::string, Material> m_materials;
    };
}
