#include "Camera.h"

Camera::Camera(const glm::vec3& pos, float y, float p)
    : position(pos), yaw(y), pitch(p)
{
    UpdateVectors();
}

void Camera::SetPosition(const glm::vec3& pos)
{
    position = pos;
}

glm::vec3 Camera::GetPosition() const
{
    return position;
}

void Camera::SetFront(const glm::vec3& frontDir)
{
    front = glm::normalize(frontDir);
    UpdateVectors();  // пересчитываем right/up
}

glm::vec3 Camera::GetFront() const
{
    return front;
}

void Camera::SetRotation(float newYaw, float newPitch)
{
    yaw = newYaw;
    pitch = glm::clamp(newPitch, -89.0f, 89.0f);  // ограничиваем, чтобы не переворачиваться
    UpdateVectors();
}

glm::vec2 Camera::GetRotation() const
{
    return glm::vec2(yaw, pitch);
}

void Camera::LookAt(const glm::vec3& target)
{
    front = glm::normalize(target - position);
    pitch = glm::degrees(glm::asin(front.y));
    yaw = glm::degrees(glm::atan(front.x, front.z));
    UpdateVectors();
}

void Camera::Move(const glm::vec3& delta)
{
    position += delta;
}

void Camera::MoveForward(float distance)
{
    position += front * distance;
}

void Camera::MoveRight(float distance)
{
    position += right * distance;
}

void Camera::MoveUp(float distance)
{
    position += up * distance;  // локальный up
    // или position += worldUp * distance; для глобального
}

void Camera::Rotate(float yawDelta, float pitchDelta)
{
    yaw += yawDelta * sensitivity;
    pitch += pitchDelta * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    UpdateVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio, float fov, float nearPlane, float farPlane) const
{
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::Update()
{
    UpdateVectors();
}

void Camera::UpdateVectors()
{
    glm::vec3 newFront;
    newFront.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    newFront.y = glm::sin(glm::radians(pitch));
    newFront.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}