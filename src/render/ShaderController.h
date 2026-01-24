// src/render/ShaderController.h
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include "log/Logger.h"

namespace ogle {

	// Предварительные объявления

		class ShaderProgram;
		class ShaderControllerImpl;
	

	class ShaderController {
	public:
		static ShaderController& Get();

		// === Основное API ===
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

		// === Встроенные шейдеры ===
		enum class Builtin { 
			BasicColor, 
			BasicTexture, 
			Skybox, 
			Unlit, 
			Wireframe
		};
		std::shared_ptr<ShaderProgram> GetBuiltin(Builtin type);
		void PreloadBuiltinShaders();

		// === Управление ===
		void RemoveProgram(const std::string& name);
		void Clear();

		// === Отладка и статистика ===
		struct Statistics {
			size_t totalPrograms = 0;
			size_t compiledShaders = 0;
			size_t cacheHits = 0;
			size_t cacheMisses = 0;
		};

		Statistics GetStats() const;
		void PrintDebugInfo() const;

		// === Hot-reload ===
		void WatchShaderFiles(bool enable);
		void CheckForUpdates();

	private:
		ShaderController();
		~ShaderController();

		std::unique_ptr<ShaderControllerImpl> m_impl;

		// Запрет копирования
		ShaderController(const ShaderController&) = delete;
		ShaderController& operator=(const ShaderController&) = delete;
	};

} // namespace ogle