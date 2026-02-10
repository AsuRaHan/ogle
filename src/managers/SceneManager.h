// src/managers/SceneManager.h
#pragma once

#include <memory>
#include <string>

namespace ogle {

class Scene;
class JsonSceneSerializer;
class Logger;

class SceneManager {
public:
    // Получить единственный экземпляр менеджера
    static SceneManager& Get();

    // Доступ к сцене
    Scene* GetScene();

    // Сохранить сцену в файл
    bool SaveScene(const std::string& filepath);

    // Загрузить сцену из файла
    bool LoadScene(const std::string& filepath);

private:
    SceneManager();
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unique_ptr<Scene> m_scene;
};

} // namespace ogle
