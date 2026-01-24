// src/render/shader/internal/ShaderLinker.cpp
#include "ShaderLinker.h"

namespace ogle {

	LinkResult ShaderLinker::Link(const std::vector<GLuint>& shaderIds,
		const std::string& programName) {
		LinkResult result;

		if (shaderIds.empty()) {
			result.infoLog = "No shaders to link";
			Logger::Error("No shaders to link for: " + programName);
			return result;
		}

		// Создаём программу
		result.programId = glCreateProgram();
		if (result.programId == 0) {
			result.infoLog = "Failed to create program";
			Logger::Error("glCreateProgram failed for: " + programName);
			return result;
		}

		// Присоединяем шейдеры
		for (GLuint shaderId : shaderIds) {
			if (shaderId != 0) {
				glAttachShader(result.programId, shaderId);
				result.attachedShaders.push_back(shaderId);
			}
		}

		// Линкуем
		glLinkProgram(result.programId);

		// Проверяем результат
		result.success = CheckLinkStatus(result.programId, result.infoLog);

		// Отсоединяем шейдеры (они больше не нужны)
		for (GLuint shaderId : result.attachedShaders) {
			glDetachShader(result.programId, shaderId);
		}

		if (result.success) {
			Logger::Info("Program linked: " +
				(programName.empty() ? "unnamed" : programName) +
				" (ID: " + std::to_string(result.programId) + ")");
		}
		else {
			Logger::Error("Program linking failed: " +
				(programName.empty() ? "unnamed" : programName) +
				"\n" + result.infoLog);

			glDeleteProgram(result.programId);
			result.programId = 0;
		}

		return result;
	}

	bool ShaderLinker::Validate(GLuint programId) {
		if (programId == 0) return false;

		GLint status;
		glValidateProgram(programId);
		glGetProgramiv(programId, GL_VALIDATE_STATUS, &status);

		return status == GL_TRUE;
	}

	bool ShaderLinker::CheckLinkStatus(GLuint programId, std::string& infoLog) {
		GLint success;
		glGetProgramiv(programId, GL_LINK_STATUS, &success);

		if (success == GL_TRUE) {
			return true;
		}

		// Получаем лог ошибки
		GLint logLength = 0;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 1) {
			std::vector<char> log(logLength);
			glGetProgramInfoLog(programId, logLength, nullptr, log.data());
			infoLog = std::string(log.data());
		}

		return false;
	}

} // namespace ogle