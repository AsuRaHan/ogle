// src/render/shader/internal/ShaderLoader.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "log/Logger.h"

namespace ogle {

	class ShaderLoader {
	public:
		ShaderLoader();

		// Основные методы
		std::string LoadFromFile(const std::string& filepath);
		std::string GetBuiltinSource(const std::string& shaderName);

		// Утилиты
		std::string Preprocess(const std::string& source,
			const std::string& basePath = "");

		bool HasFileChanged(const std::string& filepath);

	private:
		std::string ReadFile(const std::string& filepath);
		std::string ResolveIncludes(const std::string& source,
			const std::string& basePath);

		struct FileCache {
			std::string content;
			std::filesystem::file_time_type lastWrite;
		};

		std::unordered_map<std::string, FileCache> m_cache;
	};

} // namespace ogle