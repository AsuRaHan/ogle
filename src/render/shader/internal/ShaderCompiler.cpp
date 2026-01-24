// src/render/shader/internal/ShaderCompiler.cpp
#include "ShaderCompiler.h"

namespace ogle {

	CompileResult ShaderCompiler::Compile(GLenum type, const std::string& source, const std::string& shaderName) {
		CompileResult result;

		if (source.empty()) {
			result.infoLog = "Empty shader source";
			Logger::Error("Empty shader source for: " + shaderName);
			return result;
		}

		// Создаём шейдер
		result.shaderId = glCreateShader(type);
		if (result.shaderId == 0) {
			result.infoLog = "Failed to create shader object";
			Logger::Error("glCreateShader failed for: " + shaderName);
			return result;
		}

		// Компилируем
		const char* src = source.c_str();
		glShaderSource(result.shaderId, 1, &src, nullptr);
		glCompileShader(result.shaderId);

		// Проверяем результат
		result.success = CheckCompileStatus(result.shaderId, result.infoLog);

		if (result.success) {
			Logger::Info("Shader compiled: " +
				(shaderName.empty() ? "unnamed" : shaderName) +
				" [" + GetTypeName(type) + "]");
		}
		else {
			Logger::Error("Shader compilation failed: " +
				(shaderName.empty() ? "unnamed" : shaderName) +
				" [" + GetTypeName(type) + "]\n" +
				result.infoLog);

			glDeleteShader(result.shaderId);
			result.shaderId = 0;
		}

		return result;
	}

	std::string ShaderCompiler::GetTypeName(GLenum type) {
		switch (type) {
		case GL_VERTEX_SHADER: return "VERTEX";
		case GL_FRAGMENT_SHADER: return "FRAGMENT";
		case GL_GEOMETRY_SHADER: return "GEOMETRY";
		case GL_TESS_CONTROL_SHADER: return "TESS_CONTROL";
		case GL_TESS_EVALUATION_SHADER: return "TESS_EVALUATION";
		case GL_COMPUTE_SHADER: return "COMPUTE";
		default: return "UNKNOWN";
		}
	}

	bool ShaderCompiler::Validate(GLuint shaderId) {
		if (shaderId == 0) return false;

		GLint status;
		// glValidateShader(shaderId);
		glGetShaderiv(shaderId, GL_VALIDATE_STATUS, &status);
		return status == GL_TRUE;
	}

	bool ShaderCompiler::CheckCompileStatus(GLuint shaderId, std::string& infoLog) {
		GLint success;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

		if (success == GL_TRUE) {
			return true;
		}

		// Получаем лог ошибки
		GLint logLength = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 1) {
			std::vector<char> log(logLength);
			glGetShaderInfoLog(shaderId, logLength, nullptr, log.data());
			infoLog = std::string(log.data(), logLength - 1);
		}

		return false;
	}

} // namespace ogle