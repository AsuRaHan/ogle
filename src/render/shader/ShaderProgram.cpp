// src/render/shader/ShaderProgram.cpp
#include "ShaderProgram.h"

namespace ogle {

	std::shared_ptr<ShaderProgram> ShaderProgram::Create(GLuint id, const std::string& name) {
		if (id == 0) {
			Logger::Error("Cannot create ShaderProgram with invalid ID (0)");
			return nullptr;
		}

		// Используем приватный конструктор
		return std::shared_ptr<ShaderProgram>(new ShaderProgram(id, name));
	}

	ShaderProgram::ShaderProgram(GLuint id, const std::string& name)
		: m_id(id), m_name(name) {
		Logger::Debug("ShaderProgram created: " + name + " (ID: " +
			std::to_string(id) + ")");
	}

	ShaderProgram::~ShaderProgram() {
		if (m_id != 0) {
			glDeleteProgram(m_id);
			Logger::Debug("ShaderProgram destroyed: " + m_name);
		}
	}

	void ShaderProgram::Bind() const {
		if (m_id != 0) {
			glUseProgram(m_id);
		}
	}

	void ShaderProgram::Unbind() const {
		glUseProgram(0);
	}

	GLint ShaderProgram::GetUniformLocation(const std::string& name) {
		// Проверяем кэш
		auto it = m_uniformCache.find(name);
		if (it != m_uniformCache.end()) {
			return it->second;
		}

		// Получаем локацию
		GLint location = glGetUniformLocation(m_id, name.c_str());
		if (location == -1) {
			Logger::Warning("Uniform '" + name + "' not found in shader: " + m_name);
		}

		m_uniformCache[name] = location;
		return location;
	}

	void ShaderProgram::SetInt(const std::string& name, int value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniform1i(location, value);
		}
	}

	void ShaderProgram::SetFloat(const std::string& name, float value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniform1f(location, value);
		}
	}

	void ShaderProgram::SetVec2(const std::string& name, const glm::vec2& value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniform2f(location, value.x, value.y);
		}
	}

	void ShaderProgram::SetVec3(const std::string& name, const glm::vec3& value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniform3f(location, value.x, value.y, value.z);
		}
	}

	void ShaderProgram::SetVec4(const std::string& name, const glm::vec4& value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniform4f(location, value.x, value.y, value.z, value.w);
		}
	}

	void ShaderProgram::SetMat3(const std::string& name, const glm::mat3& value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
		}
	}

	void ShaderProgram::SetMat4(const std::string& name, const glm::mat4& value) {
		GLint location = GetUniformLocation(name);
		if (location != -1) {
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
		}
	}

	void ShaderProgram::SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
		SetMat4("uModel", model);
		SetMat4("uView", view);
		SetMat4("uProjection", projection);
	}

} // namespace ogle