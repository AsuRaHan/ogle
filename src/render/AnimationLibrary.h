#pragma once

#include "../world/WorldComponents.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace OGLE {
    class AnimationLibrary {
    public:
        static AnimationLibrary& Instance();

        bool AddAnimation(const std::string& name, const AnimationComponent& animation);
        bool RemoveAnimation(const std::string& name);
        AnimationComponent* GetAnimation(const std::string& name);
        const AnimationComponent* GetAnimation(const std::string& name) const;
        std::vector<std::string> GetAnimationNames() const;

        bool SaveToFile(const std::string& path) const;
        bool LoadFromFile(const std::string& path);

    private:
        AnimationLibrary() = default;
        std::unordered_map<std::string, AnimationComponent> m_animations;
    };
}
