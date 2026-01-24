// src/render/shader/internal/ShaderCache.h
#pragma once
#include <glad/gl.h>
#include <string>
#include <unordered_map>
#include "log/Logger.h"

namespace ogle {

	class ShaderCache {
	public:
		ShaderCache() = default;

		// Кэширование программ
		GLuint GetProgram(const std::string& key);
		void StoreProgram(const std::string& key, GLuint programId);
		void RemoveProgram(const std::string& key);

		// Управление
		void Clear();
		size_t GetSize() const { return m_programs.size(); }

	private:
		std::unordered_map<std::string, GLuint> m_programs;
	};

} // namespace ogle