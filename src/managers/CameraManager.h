#pragma once

#include "opengl/Camera.h"

#include <glm/vec3.hpp>

class CameraManager
{
public:
    CameraManager();

    OGLE::Camera& GetCamera();
    const OGLE::Camera& GetCamera() const;

    void Update(float deltaTime);

    void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip);
    void SetOrthographic(float size, float aspectRatio, float nearClip, float farClip);
    void SetOrthographic(float left, float right, float bottom, float top, float nearClip, float farClip);
    void SetAspectRatio(float aspectRatio);

    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::quat& rotation);
    void SetRotation(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f);
    void Translate(const glm::vec3& translation);
    void Rotate(float yawDelta, float pitchDelta, float rollDelta = 0.0f);
    void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);

    void SetOrbitTarget(const glm::vec3& target);
    void SetOrbitDistance(float distance);
    void Orbit(float horizontalAngle, float verticalAngle);

    void SetMode(OGLE::Camera::Mode mode);
    OGLE::Camera::Mode GetMode() const;

private:
    OGLE::Camera m_camera;
};
