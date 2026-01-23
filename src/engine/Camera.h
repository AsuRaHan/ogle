#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f), float yaw = -90.0f, float pitch = 0.0f);

    // Установка/получение позиции
    void SetPosition(const glm::vec3& pos);
    glm::vec3 GetPosition() const;

    // Установка/получение направления (front)
    void SetFront(const glm::vec3& frontDir);
    glm::vec3 GetFront() const;

    // Установка/получение углов вращения (в градусах)
    void SetRotation(float newYaw, float newPitch);
    glm::vec2 GetRotation() const;  // возвращает yaw, pitch

    // LookAt — смотреть на точку
    void LookAt(const glm::vec3& target);

    // Движение
    void Move(const glm::vec3& delta);  // абсолютное перемещение
    void MoveForward(float distance);   // вперёд по front
    void MoveRight(float distance);     // вправо по right
    void MoveUp(float distance);        // вверх по up (мировой или локальной — здесь мировой)

    // Вращение
    void Rotate(float yawDelta, float pitchDelta);  // дельта в градусах

    // Матрицы
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio, float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f) const;

    // Обновление (вызывать в Update камеры, если изменились углы/позиция)
    void Update();

    // Настройки
    void SetSensitivity(float newSensitivity) { sensitivity = newSensitivity; }
    float GetSensitivity() const { return sensitivity; }

private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp { 0.0f, 1.0f, 0.0f };  // глобальный up для расчёта

    float yaw;
    float pitch;

    float sensitivity = 0.1f;  // для мыши, если добавишь

    void UpdateVectors();  // пересчёт front/right/up на основе yaw/pitch
};