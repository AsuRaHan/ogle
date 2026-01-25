// src/render/Camera.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace ogle {

	class Camera {
	public:
		enum class Type {
			Perspective,
			Orthographic
		};

		enum class Mode {
			Free,          // Свободная камера (редактор)
			FirstPerson,   // От первого лица
			Orbit,         // Орбитальная вокруг цели
			Fixed          // Фиксированная
		};

		Camera(const std::string& name = "Camera");
		~Camera() = default;

		// === Основные методы ===
		void Update(float deltaTime);

		// Матрицы
		const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
		glm::mat4 GetViewProjection() const { return m_projectionMatrix * m_viewMatrix; }

		// === Управление проекцией ===
		void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip);
		void SetOrthographic(float size, float aspectRatio, float nearClip, float farClip);
		void SetOrthographic(float left, float right, float bottom, float top, float nearClip, float farClip);

		// === Трансформация ===
		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::quat& rotation);
		void SetRotation(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f);

		void Translate(const glm::vec3& translation);
		void Rotate(float yawDelta, float pitchDelta, float rollDelta = 0.0f);

		void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

		// === Движение (для FPS камеры) ===
		void MoveForward(float distance);
		void MoveRight(float distance);
		void MoveUp(float distance);

		// === Орбитальная камера ===
		void SetOrbitTarget(const glm::vec3& target);
		void SetOrbitDistance(float distance);
		void Orbit(float horizontalAngle, float verticalAngle);

		void SetAspectRatio(float aspectRatio);

		// === Геттеры ===
		const glm::vec3& GetPosition() const { return m_position; }
		const glm::vec3& GetFront() const { return m_front; }
		const glm::vec3& GetRight() const { return m_right; }
		const glm::vec3& GetUp() const { return m_up; }

		glm::quat GetRotation() const { return m_rotation; }
		float GetYaw() const { return m_yaw; }
		float GetPitch() const { return m_pitch; }
		float GetRoll() const { return m_roll; }

		Type GetType() const { return m_type; }
		void SetType(Type type) { m_type = type; }

		Mode GetMode() const { return m_mode; }
		void SetMode(Mode mode) { m_mode = mode; }

		const std::string& GetName() const { return m_name; }

		// === Настройки ===
		void SetMovementSpeed(float speed) { m_movementSpeed = speed; }
		void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }
		void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

		float GetMovementSpeed() const { return m_movementSpeed; }
		float GetRotationSpeed() const { return m_rotationSpeed; }
		float GetMouseSensitivity() const { return m_mouseSensitivity; }

		// === Для InputSystem ===
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		void ProcessMouseScroll(float yoffset);

	private:
		void UpdateCameraVectors();
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

		std::string m_name;

		// Трансформация
		glm::vec3 m_position = { 0.0f, 0.0f, 5.0f };
		glm::quat m_rotation = glm::identity<glm::quat>();

		// Углы Эйлера (для FPS камеры)
		float m_yaw = -90.0f;    // Вращение вокруг Y
		float m_pitch = 0.0f;    // Вращение вокруг X
		float m_roll = 0.0f;     // Вращение вокруг Z

		// Векторы направления
		glm::vec3 m_front = { 0.0f, 0.0f, -1.0f };
		glm::vec3 m_right = { 1.0f, 0.0f, 0.0f };
		glm::vec3 m_up = { 0.0f, 1.0f, 0.0f };
		glm::vec3 m_worldUp = { 0.0f, 1.0f, 0.0f };

		// Матрицы
		glm::mat4 m_viewMatrix = glm::mat4(1.0f);
		glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

		// Тип камеры
		Type m_type = Type::Perspective;
		Mode m_mode = Mode::Free;

		// Параметры проекции
		union {
			struct {
				float fov;          // в градусах
				float aspectRatio;
				float nearClip;
				float farClip;
			} perspective;

			struct {
				float size;         // Высота видимой области
				float aspectRatio;
				float nearClip;
				float farClip;
			} orthographic;
		} m_projectionParams;

		// Настройки управления
		float m_movementSpeed = 5.0f;
		float m_rotationSpeed = 0.1f;
		float m_mouseSensitivity = 0.1f;
		float m_zoom = 45.0f;       // FOV для зума

		// Для орбитальной камеры
		glm::vec3 m_orbitTarget = { 0.0f, 0.0f, 0.0f };
		float m_orbitDistance = 10.0f;
		float m_orbitHorizontal = 0.0f;
		float m_orbitVertical = 30.0f;

		// Флаги
		bool m_viewDirty = true;
		bool m_projectionDirty = true;
	};

} // namespace ogle