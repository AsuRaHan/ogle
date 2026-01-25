// src/managers/CameraManager.cpp
#include "CameraManager.h"
#include "log/Logger.h"

namespace ogle {

CameraManager::CameraManager() {
    Logger::Info("CameraManager initialized");
}

Camera* CameraManager::CreateCamera(const std::string& name) {
    // Проверяем, нет ли уже камеры с таким именем
    auto it = m_cameras.find(name);
    if (it != m_cameras.end()) {
        Logger::Warning("Camera already exists: " + name);
        return it->second.get();
    }
    
    // Создаем новую камеру
    auto camera = std::make_unique<Camera>(name);
    Camera* ptr = camera.get();
    m_cameras[name] = std::move(camera);
    
    // Если это первая камера - запоминаем как основную
    if (!m_mainCamera) {
        m_mainCamera = ptr;
    }
    
    Logger::Info("Camera created: " + name);
    return ptr;
}

Camera* CameraManager::GetCamera(const std::string& name) {
    auto it = m_cameras.find(name);
    if (it != m_cameras.end()) {
        return it->second.get();
    }
    
    Logger::Warning("Camera not found: " + name);
    return nullptr;
}

Camera* CameraManager::GetMainCamera() {
    return m_mainCamera;
}

bool CameraManager::RemoveCamera(const std::string& name) {
    auto it = m_cameras.find(name);
    if (it != m_cameras.end()) {
        // Если удаляем основную камеру и она не последняя
        if (it->second.get() == m_mainCamera && m_cameras.size() > 1) {
            // Находим другую камеру для замены
            for (auto& pair : m_cameras) {
                if (pair.first != name) {
                    m_mainCamera = pair.second.get();
                    break;
                }
            }
        }
        
        m_cameras.erase(it);
        
        // Если это была последняя камера
        if (m_cameras.empty()) {
            m_mainCamera = nullptr;
        }
        
        Logger::Info("Camera removed: " + name);
        return true;
    }
    
    return false;
}

} // namespace ogle