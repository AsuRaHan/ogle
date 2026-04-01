#pragma once

class CameraManager;

class InputActionsManager
{
public:
    void ConfigureDefaultActions();
    void UpdateCameraControls(CameraManager& cameraManager, float deltaTime);

private:
    bool m_isConfigured = false;
};
