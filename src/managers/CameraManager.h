// src/managers/CameraManager.h
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "render/Camera.h"

namespace ogle {

class CameraManager {
public:
    static CameraManager& Get() {
        static CameraManager instance;
        return instance;
    }
    
    // === Только три метода ===
    
    // Создать камеру
    Camera* CreateCamera(const std::string& name = "MainCamera");
    
    // Получить камеру по имени
    Camera* GetCamera(const std::string& name = "MainCamera");
    
    // Получить основную камеру (первая созданная)
    Camera* GetMainCamera();
    
    // Удалить камеру
    bool RemoveCamera(const std::string& name);
    
    // Получить количество камер
    size_t GetCameraCount() const { return m_cameras.size(); }
    
private:
    CameraManager();
    ~CameraManager() = default;
    
    // Запрет копирования
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    
    // Хранилище камер
    std::unordered_map<std::string, std::unique_ptr<Camera>> m_cameras;
    
    // Указатель на первую/основную камеру
    Camera* m_mainCamera = nullptr;
};

} // namespace ogle