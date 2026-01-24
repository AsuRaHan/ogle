// src/render/shader/internal/ShaderCompiler.h
#pragma once
#include <glad/gl.h>
#include <string>
#include <vector>
#include "log/Logger.h"

namespace ogle {

	struct CompileResult {
		GLuint shaderId = 0;
		bool success = false;
		std::string infoLog;
	};

	class ShaderCompiler {
	public:
		ShaderCompiler() = default;

		CompileResult Compile(GLenum type, const std::string& source,
			const std::string& shaderName = "");

		// Утилиты
		static std::string GetTypeName(GLenum type);
		static bool Validate(GLuint shaderId);

	private:
		bool CheckCompileStatus(GLuint shaderId, std::string& infoLog);
	};

} // namespace ogle