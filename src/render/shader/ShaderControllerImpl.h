// src/render/shader/ShaderControllerImpl.h
#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include "log/Logger.h"
#include "ShaderProgram.h"
#include "internal/ShaderLoader.h"
#include "internal/ShaderCompiler.h"
#include "internal/ShaderLinker.h"
#include "internal/ShaderCache.h"

namespace ogle {

	struct ShaderProgramInfo {
		std::shared_ptr<ShaderProgram> program;
		std::string vertexPath;
		std::string fragmentPath;
		std::string geometryPath;
		std::string vertexSource;
		std::string fragmentSource;
		std::string geometrySource;
		std::filesystem::file_time_type lastModified;
	};

	// Приватная реализация ShaderController
	class ShaderControllerImpl {
	public:
		ShaderControllerImpl();
		~ShaderControllerImpl();

		// Основные методы (копия публичного API)
		std::shared_ptr<ShaderProgram> CreateProgram(const std::string& name,
			const std::string& vertexSource,
			const std::string& fragmentSource,
			const std::string& geometrySource = "");

		std::shared_ptr<ShaderProgram> CreateProgramFromFiles(const std::string& name,
			const std::string& vertexPath,
			const std::string& fragmentPath,
			const std::string& geometryPath = "");

		std::shared_ptr<ShaderProgram> GetProgram(const std::string& name);
		bool HasProgram(const std::string& name) const;

		void UseProgram(const std::string& name);
		void UseProgram(std::shared_ptr<ShaderProgram> program);
		std::shared_ptr<ShaderProgram> GetCurrentProgram() const;

		// Встроенные шейдеры
		enum class Builtin { BasicColor, BasicTexture, Skybox, Unlit, Wireframe };
		std::shared_ptr<ShaderProgram> GetBuiltin(Builtin type);
		void PreloadBuiltinShaders();

		// Управление
		void RemoveProgram(const std::string& name);
		void Clear();

		// Статистика
		struct Statistics {
			size_t totalPrograms = 0;
			size_t compiledShaders = 0;
			size_t cacheHits = 0;
			size_t cacheMisses = 0;
		};

		Statistics GetStats() const;
		void PrintDebugInfo() const;

		// Hot-reload
		void WatchShaderFiles(bool enable);
		void CheckForUpdates();

	private:
		// Вспомогательные методы
		void CleanupShaders(const std::vector<GLuint>& shaderIds);
		std::string GenerateCacheKey(const std::string& name,
			const std::string& vertexSource,
			const std::string& fragmentSource,
			const std::string& geometrySource);

		std::string GetBuiltinName(Builtin type);
		void GetBuiltinSources(Builtin type,
			std::string& outVertex,
			std::string& outFragment);

		// Компоненты
		ShaderLoader m_loader;
		ShaderCompiler m_compiler;
		ShaderLinker m_linker;
		ShaderCache m_cache;

		// Хранилище
		std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_programs;
		std::shared_ptr<ShaderProgram> m_currentProgram;

		Statistics m_stats;
		bool m_watchFiles = false;

		std::unordered_map<std::string, ShaderProgramInfo> m_programInfos;
		bool m_watchEnabled = true;
		float m_checkInterval = 1.0f; // Проверять каждую секунду
		float m_timeSinceLastCheck = 0.0f;
	};

} // namespace ogle