// src/managers/SceneManager.cpp
#include "SceneManager.h"
#include "scene/Scene.h"
#include "scene/JsonSceneSerializer.h"
#include "log/Logger.h"

namespace ogle {

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

Scene* SceneManager::GetScene() {
    return m_scene.get();
}

bool SceneManager::SaveScene(const std::string& filepath) {
    if (!m_scene) {
        Logger::Error("SceneManager: No scene to save");
        return false;
    }
    bool res = JsonSceneSerializer::SaveToFile(*m_scene, filepath);
    if (res) {
        Logger::Info("Scene saved to " + filepath);
    } else {
        Logger::Error("Failed to save scene to " + filepath);
    }
    return res;
}

bool SceneManager::LoadScene(const std::string& filepath) {
    if (!m_scene) {
        m_scene = std::make_unique<Scene>();
    }
    bool res = JsonSceneSerializer::LoadFromFile(*m_scene, filepath);
    if (res) {
        Logger::Info("Scene loaded from " + filepath);
    } else {
        Logger::Error("Failed to load scene from " + filepath);
    }
    return res;
}

SceneManager::SceneManager() {
    m_scene = std::make_unique<Scene>();
    Logger::Info("SceneManager initialized");
}

} // namespace ogle
