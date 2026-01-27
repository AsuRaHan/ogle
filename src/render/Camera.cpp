// src/render/Camera.cpp
#include "Camera.h"
#include <algorithm>

namespace ogle {

	Camera::Camera(const std::string& name)
		: m_name(name) {
		SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
		UpdateCameraVectors();
	}

	void Camera::Update(float deltaTime) {
		// Обновление орбитальной камеры
		if (m_mode == Mode::Orbit) {
			float horizontalRad = glm::radians(m_orbitHorizontal);
			float verticalRad = glm::radians(m_orbitVertical);

			m_position.x = m_orbitTarget.x + m_orbitDistance * cosf(verticalRad) * sinf(horizontalRad);
			m_position.y = m_orbitTarget.y + m_orbitDistance * sinf(verticalRad);
			m_position.z = m_orbitTarget.z + m_orbitDistance * cosf(verticalRad) * cosf(horizontalRad);

			LookAt(m_orbitTarget);
		}

		// Обновление матриц при необходимости
		if (m_viewDirty) {
			UpdateViewMatrix();
		}

		if (m_projectionDirty) {
			UpdateProjectionMatrix();
		}

		if (m_viewDirty || m_projectionDirty) {
			ExtractFrustumPlanes();
		}
	}

	void Camera::SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip) {
		m_type = Type::Perspective;
		m_projectionParams.perspective.fov = fovDegrees;
		m_projectionParams.perspective.aspectRatio = aspectRatio;
		m_projectionParams.perspective.nearClip = nearClip;
		m_projectionParams.perspective.farClip = farClip;
		m_projectionDirty = true;
	}

	void Camera::SetOrthographic(float size, float aspectRatio, float nearClip, float farClip) {
		m_type = Type::Orthographic;
		float halfHeight = size * 0.5f;
		float halfWidth = halfHeight * aspectRatio;

		m_projectionParams.orthographic.size = size;
		m_projectionParams.orthographic.aspectRatio = aspectRatio;
		m_projectionParams.orthographic.nearClip = nearClip;
		m_projectionParams.orthographic.farClip = farClip;
		m_projectionDirty = true;
	}

	void Camera::SetOrthographic(float left, float right, float bottom, float top,
		float nearClip, float farClip) {
		m_type = Type::Orthographic;
		m_projectionDirty = true;
	}

	void Camera::SetPosition(const glm::vec3& position) {
		m_position = position;
		m_viewDirty = true;
	}

	void Camera::SetRotation(const glm::quat& rotation) {
		m_rotation = rotation;
		// Извлекаем углы Эйлера из кватерниона
		glm::vec3 euler = glm::eulerAngles(rotation);
		m_yaw = glm::degrees(euler.y);
		m_pitch = glm::degrees(euler.x);
		m_roll = glm::degrees(euler.z);
		UpdateCameraVectors();
	}

	void Camera::SetRotation(float yawDegrees, float pitchDegrees, float rollDegrees) {
		m_yaw = yawDegrees;
		m_pitch = pitchDegrees;
		m_roll = rollDegrees;

		// Создаем кватернион из углов Эйлера
		glm::quat qYaw = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat qPitch = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat qRoll = glm::angleAxis(glm::radians(m_roll), glm::vec3(0.0f, 0.0f, 1.0f));

		m_rotation = qYaw * qPitch * qRoll;
		UpdateCameraVectors();
	}

	void Camera::Translate(const glm::vec3& translation) {
		m_position += translation;
		m_viewDirty = true;
	}

	void Camera::Rotate(float yawDelta, float pitchDelta, float rollDelta) {
		//Logger::Debug("Camera::Rotate - Delta: " +
		//	std::to_string(yawDelta) + ", " +
		//	std::to_string(pitchDelta) +
		//	" Current: " + std::to_string(m_yaw) + ", " +
		//	std::to_string(m_pitch));
		m_yaw += yawDelta * m_rotationSpeed;
		m_pitch += pitchDelta * m_rotationSpeed;
		m_roll += rollDelta * m_rotationSpeed;

		// Ограничиваем pitch, чтобы не переворачивать камеру
		m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

		SetRotation(m_yaw, m_pitch, m_roll);
	}

	void Camera::LookAt(const glm::vec3& target, const glm::vec3& up) {
		m_viewMatrix = glm::lookAt(m_position, target, up);

		// Извлекаем фронт из матрицы
		m_front = glm::normalize(target - m_position);
		m_right = glm::normalize(glm::cross(m_front, up));
		m_up = glm::normalize(glm::cross(m_right, m_front));

		// Обновляем углы
		m_pitch = glm::degrees(asin(m_front.y));
		m_yaw = glm::degrees(atan2(m_front.x, m_front.z));

		m_viewDirty = false;
	}

	void Camera::MoveForward(float distance) {
		m_position += m_front * distance;
		m_viewDirty = true;
	}

	void Camera::MoveRight(float distance) {
		m_position += m_right * distance;
		m_viewDirty = true;
	}

	void Camera::MoveUp(float distance) {
		m_position += m_up * distance;
		m_viewDirty = true;
	}

	void Camera::SetOrbitTarget(const glm::vec3& target) {
		m_orbitTarget = target;
		m_mode = Mode::Orbit;
	}

	void Camera::SetOrbitDistance(float distance) {
		m_orbitDistance = std::max(1.0f, distance);
	}

	void Camera::Orbit(float horizontalAngle, float verticalAngle) {
		m_orbitHorizontal += horizontalAngle * m_rotationSpeed;
		m_orbitVertical = std::clamp(m_orbitVertical + verticalAngle * m_rotationSpeed, -89.0f, 89.0f);
	}

	void Camera::SetAspectRatio(float aspectRatio) {
		if (m_type == Type::Perspective) {
			m_projectionParams.perspective.aspectRatio = aspectRatio;
			m_projectionDirty = true;
		}
		else {
			m_projectionParams.orthographic.aspectRatio = aspectRatio;
			m_projectionDirty = true;
		}
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
		if (m_mode == Mode::Free || m_mode == Mode::FirstPerson) {
			Rotate(xoffset * m_mouseSensitivity, yoffset * m_mouseSensitivity);
		}
		else if (m_mode == Mode::Orbit) {
			Orbit(xoffset * m_mouseSensitivity, -yoffset * m_mouseSensitivity);
		}
	}

	void Camera::ProcessMouseScroll(float yoffset) {
		if (m_type == Type::Perspective) {
			m_projectionParams.perspective.fov = std::clamp(
				m_projectionParams.perspective.fov - yoffset,
				1.0f, 120.0f
			);
			m_projectionDirty = true;
		}
		else if (m_mode == Mode::Orbit) {
			SetOrbitDistance(m_orbitDistance - yoffset);
		}
	}

	void Camera::UpdateCameraVectors() {
		// Вычисляем новые векторы из углов Эйлера
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		newFront.y = sin(glm::radians(m_pitch));
		newFront.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

		//Logger::Debug("UpdateCameraVectors - Front: " +
		//	std::to_string(newFront.x) + ", " +
		//	std::to_string(newFront.y) + ", " +
		//	std::to_string(newFront.z));

		m_front = glm::normalize(newFront);
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));

		m_viewDirty = true;
	}

	void Camera::UpdateViewMatrix() {
		if (m_mode != Mode::Orbit) { // Орбитальная обновляется в Update
			m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
		}
		m_viewDirty = false;
	}

	void Camera::UpdateProjectionMatrix() {
		if (m_type == Type::Perspective) {
			m_projectionMatrix = glm::perspective(
				glm::radians(m_projectionParams.perspective.fov),
				m_projectionParams.perspective.aspectRatio,
				m_projectionParams.perspective.nearClip,
				m_projectionParams.perspective.farClip
			);
		}
		else {
			float halfHeight = m_projectionParams.orthographic.size * 0.5f;
			float halfWidth = halfHeight * m_projectionParams.orthographic.aspectRatio;

			m_projectionMatrix = glm::ortho(
				-halfWidth, halfWidth,
				-halfHeight, halfHeight,
				m_projectionParams.orthographic.nearClip,
				m_projectionParams.orthographic.farClip
			);
		}
		m_projectionDirty = false;
	}

	bool Camera::IsInFrustum(const glm::vec3& position, float radius) const {
		// Проверяем сферу против 6 плоскостей
		for (int i = 0; i < 6; ++i) {
			const glm::vec4& plane = m_frustumPlanes[i];
			float distance = glm::dot(glm::vec3(plane), position) + plane.w;

			if (distance < -radius) {
				return false;  // Сфера полностью за плоскостью
			}
		}
		return true;  // Сфера внутри или пересекает frustum
	}

	void Camera::ExtractFrustumPlanes() {
		// Комбинированная матрица clip = projection * view
		glm::mat4 clip = m_projectionMatrix * m_viewMatrix;

		// Извлекаем плоскости (нормализованные)
		// Left
		m_frustumPlanes[0] = glm::normalize(glm::vec4(
			clip[0][3] + clip[0][0],
			clip[1][3] + clip[1][0],
			clip[2][3] + clip[2][0],
			clip[3][3] + clip[3][0]
		));

		// Right
		m_frustumPlanes[1] = glm::normalize(glm::vec4(
			clip[0][3] - clip[0][0],
			clip[1][3] - clip[1][0],
			clip[2][3] - clip[2][0],
			clip[3][3] - clip[3][0]
		));

		// Bottom
		m_frustumPlanes[2] = glm::normalize(glm::vec4(
			clip[0][3] + clip[0][1],
			clip[1][3] + clip[1][1],
			clip[2][3] + clip[2][1],
			clip[3][3] + clip[3][1]
		));

		// Top
		m_frustumPlanes[3] = glm::normalize(glm::vec4(
			clip[0][3] - clip[0][1],
			clip[1][3] - clip[1][1],
			clip[2][3] - clip[2][1],
			clip[3][3] - clip[3][1]
		));

		// Near
		m_frustumPlanes[4] = glm::normalize(glm::vec4(
			clip[0][3] + clip[0][2],
			clip[1][3] + clip[1][2],
			clip[2][3] + clip[2][2],
			clip[3][3] + clip[3][2]
		));

		// Far
		m_frustumPlanes[5] = glm::normalize(glm::vec4(
			clip[0][3] - clip[0][2],
			clip[1][3] - clip[1][2],
			clip[2][3] - clip[2][2],
			clip[3][3] - clip[3][2]
		));
	}
} // namespace ogle