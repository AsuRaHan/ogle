// src/render/shader/internal/ShaderLinker.h
#pragma once
#include <glad/gl.h>
#include <vector>
#include <string>
#include "log/Logger.h"

namespace ogle {

	struct LinkResult {
		GLuint programId = 0;
		bool success = false;
		std::string infoLog;
		std::vector<GLuint> attachedShaders;
	};

	class ShaderLinker {
	public:
		ShaderLinker() = default;

		LinkResult Link(const std::vector<GLuint>& shaderIds,
			const std::string& programName = "");

		bool Validate(GLuint programId);

	private:
		bool CheckLinkStatus(GLuint programId, std::string& infoLog);
	};

} // namespace ogle