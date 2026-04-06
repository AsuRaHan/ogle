#include "managers/CameraManager.h"

CameraManager::CameraManager()
    : m_camera("MainCamera")
{
}

OGLE::Camera& CameraManager::GetCamera()
{
    return m_camera;
}

const OGLE::Camera& CameraManager::GetCamera() const
{
    return m_camera;
}

void CameraManager::Update(float deltaTime)
{
    m_camera.Update(deltaTime);
}

void CameraManager::SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip)
{
    m_camera.SetPerspective(fovDegrees, aspectRatio, nearClip, farClip);
}

void CameraManager::SetOrthographic(float size, float aspectRatio, float nearClip, float farClip)
{
    m_camera.SetOrthographic(size, aspectRatio, nearClip, farClip);
}

void CameraManager::SetOrthographic(float left, float right, float bottom, float top, float nearClip, float farClip)
{
    m_camera.SetOrthographic(left, right, bottom, top, nearClip, farClip);
}

void CameraManager::SetAspectRatio(float aspectRatio)
{
    m_camera.SetAspectRatio(aspectRatio);
}

void CameraManager::SetPosition(const glm::vec3& position)
{
    m_camera.SetPosition(position);
}

void CameraManager::SetRotation(const glm::quat& rotation)
{
    m_camera.SetRotation(rotation);
}

void CameraManager::SetRotation(float yawDegrees, float pitchDegrees, float rollDegrees)
{
    m_camera.SetRotation(yawDegrees, pitchDegrees, rollDegrees);
}

void CameraManager::Translate(const glm::vec3& translation)
{
    m_camera.Translate(translation);
}

void CameraManager::Rotate(float yawDelta, float pitchDelta, float rollDelta)
{
    m_camera.Rotate(yawDelta, pitchDelta, rollDelta);
}

void CameraManager::LookAt(const glm::vec3& target, const glm::vec3& up)
{
    m_camera.LookAt(target, up);
}

void CameraManager::MoveForward(float distance)
{
    m_camera.MoveForward(distance);
}

void CameraManager::MoveRight(float distance)
{
    m_camera.MoveRight(distance);
}

void CameraManager::MoveUp(float distance)
{
    m_camera.MoveUp(distance);
}

void CameraManager::SetOrbitTarget(const glm::vec3& target)
{
    m_camera.SetOrbitTarget(target);
}

void CameraManager::SetOrbitDistance(float distance)
{
    m_camera.SetOrbitDistance(distance);
}

void CameraManager::Orbit(float horizontalAngle, float verticalAngle)
{
    m_camera.Orbit(horizontalAngle, verticalAngle);
}

void CameraManager::SetMode(OGLE::Camera::Mode mode)
{
    m_camera.SetMode(mode);
}

OGLE::Camera::Mode CameraManager::GetMode() const
{
    return m_camera.GetMode();
}
