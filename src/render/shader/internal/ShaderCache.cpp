// src/render/shader/internal/ShaderCache.cpp
#include "ShaderCache.h"

namespace ogle {

	GLuint ShaderCache::GetProgram(const std::string& key) {
		auto it = m_programs.find(key);
		if (it != m_programs.end()) {
			return it->second;
		}
		return 0;
	}

	void ShaderCache::StoreProgram(const std::string& key, GLuint programId) {
		if (programId == 0) return;

		// Удаляем старую программу, если есть
		RemoveProgram(key);

		m_programs[key] = programId;
		Logger::Debug("Program cached: " + key + " (ID: " + std::to_string(programId) + ")");
	}

	void ShaderCache::RemoveProgram(const std::string& key) {
		auto it = m_programs.find(key);
		if (it != m_programs.end()) {
			glDeleteProgram(it->second);
			m_programs.erase(it);
			Logger::Debug("Program removed from cache: " + key);
		}
	}

	void ShaderCache::Clear() {
		for (auto& pair : m_programs) {
			glDeleteProgram(pair.second);
		}
		m_programs.clear();
		Logger::Info("Shader cache cleared");
	}

} // namespace ogle