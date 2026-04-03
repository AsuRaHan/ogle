#include "render/AnimationLibrary.h"
#include "core/FileSystem.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include "Logger.h"

namespace OGLE {

    AnimationLibrary& AnimationLibrary::Instance() {
        static AnimationLibrary instance;
        return instance;
    }

    bool AnimationLibrary::AddAnimation(const std::string& name, const AnimationComponent& animation) {
        if (name.empty()) {
            return false;
        }
        m_animations[name] = animation;
        return true;
    }

    bool AnimationLibrary::RemoveAnimation(const std::string& name) {
        return m_animations.erase(name) > 0;
    }

    AnimationComponent* AnimationLibrary::GetAnimation(const std::string& name) {
        auto it = m_animations.find(name);
        if (it == m_animations.end()) {
            return nullptr;
        }
        return &it->second;
    }

    const AnimationComponent* AnimationLibrary::GetAnimation(const std::string& name) const {
        auto it = m_animations.find(name);
        if (it == m_animations.end()) {
            return nullptr;
        }
        return &it->second;
    }

    std::vector<std::string> AnimationLibrary::GetAnimationNames() const {
        std::vector<std::string> names;
        names.reserve(m_animations.size());
        for (const auto& kv : m_animations) {
            names.push_back(kv.first);
        }
        return names;
    }

    bool AnimationLibrary::SaveToFile(const std::string& path) const {
        nlohmann::json root;
        for (const auto& kv : m_animations) {
            const auto& a = kv.second;
            root[kv.first] = {
                {"enabled", a.enabled},
                {"playing", a.playing},
                {"loop", a.loop},
                {"currentTime", a.currentTime},
                {"playbackSpeed", a.playbackSpeed},
                {"currentClip", a.currentClip}
            };
        }

        std::filesystem::path filePath(path);
        if (!FileSystem::EnsureParentDirectory(filePath)) {
            return false;
        }

        std::string content = root.dump(4);
        return FileSystem::WriteTextFile(filePath, content);
    }

    bool AnimationLibrary::LoadFromFile(const std::string& path) {
        std::filesystem::path filePath(path);
        if (!FileSystem::Exists(filePath)) {
            return false;
        }

        std::string content;
        if (!FileSystem::ReadTextFile(filePath, content)) {
            return false;
        }

        nlohmann::json root;
        try {
            root = nlohmann::json::parse(content);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse animation library JSON: " + std::string(e.what()));
            return false;
        }

        if (!root.is_object()) {
            return false;
        }

        m_animations.clear();
        for (auto it = root.begin(); it != root.end(); ++it) {
            const std::string name = it.key();
            const nlohmann::json& animJson = it.value();
            AnimationComponent anim;
            anim.enabled = animJson.value("enabled", false);
            anim.playing = animJson.value("playing", false);
            anim.loop = animJson.value("loop", true);
            anim.currentTime = animJson.value("currentTime", 0.0f);
            anim.playbackSpeed = animJson.value("playbackSpeed", 1.0f);
            anim.duration = animJson.value("duration", 1.0f);
            anim.currentClip = animJson.value("currentClip", std::string());
            m_animations[name] = anim;
        }

        return true;
    }
}
