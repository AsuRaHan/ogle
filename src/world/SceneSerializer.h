#pragma once

#include <string>

namespace OGLE {
    class World; // Forward declaration

    class SceneSerializer {
    public:
        explicit SceneSerializer(World& world);

        void Save(const std::string& path);
        void Load(const std::string& path);
    private:
        World& m_world;
    };
}