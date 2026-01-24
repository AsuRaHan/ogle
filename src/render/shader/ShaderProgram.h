// src/render/shader/ShaderProgram.h
#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>
#include "log/Logger.h"

namespace ogle {

	// Готовая шейдерная программа
	class ShaderProgram {
	public:
		~ShaderProgram();

		// Фабричный метод вместо конструктора
		static std::shared_ptr<ShaderProgram> Create(GLuint id, const std::string& name);

		// Использование
		void Bind() const;
		void Unbind() const;

		// Uniform-ы
		void SetInt(const std::string& name, int value);
		void SetFloat(const std::string& name, float value);
		void SetVec2(const std::string& name, const glm::vec2& value);
		void SetVec3(const std::string& name, const glm::vec3& value);
		void SetVec4(const std::string& name, const glm::vec4& value);
		void SetMat3(const std::string& name, const glm::mat3& value);
		void SetMat4(const std::string& name, const glm::mat4& value);

		// Установка MVP матриц за один вызов
		void SetMVP(const glm::mat4& model,
			const glm::mat4& view,
			const glm::mat4& projection);

		// Геттеры
		GLuint GetID() const { return m_id; }
		const std::string& GetName() const { return m_name; }
		bool IsValid() const { return m_id != 0; }

	private:
		// Приватный конструктор
		ShaderProgram(GLuint id, const std::string& name);
		GLint GetUniformLocation(const std::string& name);

		GLuint m_id = 0;
		std::string m_name;
		std::unordered_map<std::string, GLint> m_uniformCache;
	};

} // namespace ogle