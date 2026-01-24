// src/render/shader/ShaderControllerImpl.cpp
#include "ShaderControllerImpl.h"

namespace fs = std::filesystem;

namespace ogle {

	ShaderControllerImpl::ShaderControllerImpl() {
		Logger::Info("ShaderControllerImpl initialized");
		PreloadBuiltinShaders();
	}

	ShaderControllerImpl::~ShaderControllerImpl() {
		Clear();
		Logger::Info("ShaderControllerImpl shutdown");
	}

	std::shared_ptr<ShaderProgram> ShaderControllerImpl::CreateProgram(
		const std::string& name,
		const std::string& vertexSource,
		const std::string& fragmentSource,
		const std::string& geometrySource) {

		// Проверяем, есть ли уже такая программа
		auto it = m_programs.find(name);
		if (it != m_programs.end()) {
			Logger::Warning("Shader program already exists: " + name);
			return it->second;
		}

		// 1. Проверяем кэш
		std::string cacheKey = GenerateCacheKey(name, vertexSource,
			fragmentSource, geometrySource);
		GLuint cachedId = m_cache.GetProgram(cacheKey);
		if (cachedId != 0) {
			auto program = ShaderProgram::Create(cachedId, name);  // ← Используем фабричный метод
			m_programs[name] = program;
			m_stats.cacheHits++;
			Logger::Info("Shader program loaded from cache: " + name);
			return program;
		}

		m_stats.cacheMisses++;

		// 2. Компилируем шейдеры
		std::vector<GLuint> shaderIds;

		// Вершинный шейдер
		auto vertexResult = m_compiler.Compile(GL_VERTEX_SHADER, vertexSource,
			name + "_vertex");
		if (!vertexResult.success) {
			Logger::Error("Failed to compile vertex shader for: " + name);
			return nullptr;
		}
		shaderIds.push_back(vertexResult.shaderId);
		m_stats.compiledShaders++;

		// Фрагментный шейдер
		auto fragmentResult = m_compiler.Compile(GL_FRAGMENT_SHADER, fragmentSource,
			name + "_fragment");
		if (!fragmentResult.success) {
			CleanupShaders(shaderIds);
			Logger::Error("Failed to compile fragment shader for: " + name);
			return nullptr;
		}
		shaderIds.push_back(fragmentResult.shaderId);
		m_stats.compiledShaders++;

		// Геометрический шейдер (если есть)
		if (!geometrySource.empty()) {
			auto geometryResult = m_compiler.Compile(GL_GEOMETRY_SHADER, geometrySource,
				name + "_geometry");
			if (!geometryResult.success) {
				CleanupShaders(shaderIds);
				Logger::Error("Failed to compile geometry shader for: " + name);
				return nullptr;
			}
			shaderIds.push_back(geometryResult.shaderId);
			m_stats.compiledShaders++;
		}

		// 3. Линкуем программу
		auto linkResult = m_linker.Link(shaderIds, name);
		if (!linkResult.success) {
			CleanupShaders(shaderIds);
			Logger::Error("Failed to link shader program: " + name);
			return nullptr;
		}

		// 4. Очищаем шейдеры (они больше не нужны)
		CleanupShaders(shaderIds);

		// 5. Создаём объект программы
		auto program = ShaderProgram::Create(linkResult.programId, name);  // ← Используем фабричный метод
		m_programs[name] = program;

		// 6. Кэшируем
		m_cache.StoreProgram(cacheKey, linkResult.programId);

		m_stats.totalPrograms++;
		Logger::Info("Shader program created: " + name +
			" (ID: " + std::to_string(linkResult.programId) + ")");

		return program;
	}

	std::shared_ptr<ShaderProgram> ShaderControllerImpl::CreateProgramFromFiles(
		const std::string& name,
		const std::string& vertexPath,
		const std::string& fragmentPath,
		const std::string& geometryPath) {

		// Загружаем исходники из файлов
		std::string vertexSource = m_loader.LoadFromFile(vertexPath);
		std::string fragmentSource = m_loader.LoadFromFile(fragmentPath);

		if (vertexSource.empty() || fragmentSource.empty()) {
			Logger::Error("Failed to load shader files for: " + name);
			return nullptr;
		}

		// Предварительная обработка
		vertexSource = m_loader.Preprocess(vertexSource, vertexPath);
		fragmentSource = m_loader.Preprocess(fragmentSource, fragmentPath);

		std::string geometrySource;
		if (!geometryPath.empty()) {
			geometrySource = m_loader.LoadFromFile(geometryPath);
			if (!geometrySource.empty()) {
				geometrySource = m_loader.Preprocess(geometrySource, geometryPath);
			}
		}

		// Создаём программу
		return CreateProgram(name, vertexSource, fragmentSource, geometrySource);
	}

	std::shared_ptr<ShaderProgram> ShaderControllerImpl::GetProgram(const std::string& name) {
		auto it = m_programs.find(name);
		return it != m_programs.end() ? it->second : nullptr;
	}

	bool ShaderControllerImpl::HasProgram(const std::string& name) const {
		return m_programs.find(name) != m_programs.end();
	}

	void ShaderControllerImpl::UseProgram(const std::string& name) {
		auto program = GetProgram(name);
		if (program) {
			program->Bind();
			m_currentProgram = program;
		}
	}

	void ShaderControllerImpl::UseProgram(std::shared_ptr<ShaderProgram> program) {
		if (program) {
			program->Bind();
			m_currentProgram = program;
		}
	}

	std::shared_ptr<ShaderProgram> ShaderControllerImpl::GetCurrentProgram() const {
		return m_currentProgram;
	}

	std::shared_ptr<ShaderProgram> ShaderControllerImpl::GetBuiltin(Builtin type) {
		// Проверяем, есть ли уже в кэше
		std::string builtinName = GetBuiltinName(type);
		auto it = m_programs.find(builtinName);
		if (it != m_programs.end()) {
			return it->second;
		}

		// Создаём встроенный шейдер
		std::string vertexSource, fragmentSource;
		GetBuiltinSources(type, vertexSource, fragmentSource);

		auto program = CreateProgram(builtinName, vertexSource, fragmentSource, "");
		if (program) {
			Logger::Info("Builtin shader created: " + builtinName);
		}

		return program;
	}

	void ShaderControllerImpl::PreloadBuiltinShaders() {
		Logger::Info("Preloading builtin shaders...");

		GetBuiltin(Builtin::BasicColor);
		GetBuiltin(Builtin::BasicTexture);
		GetBuiltin(Builtin::Skybox);
		GetBuiltin(Builtin::Unlit);
		GetBuiltin(Builtin::Wireframe);

		Logger::Info("Builtin shaders preloaded");
	}

	void ShaderControllerImpl::RemoveProgram(const std::string& name) {
		auto it = m_programs.find(name);
		if (it != m_programs.end()) {
			m_programs.erase(it);

			if (m_currentProgram == it->second) {
				m_currentProgram.reset();
			}

			Logger::Info("Shader program removed: " + name);
			m_stats.totalPrograms--;
		}
	}

	void ShaderControllerImpl::Clear() {
		m_programs.clear();
		m_currentProgram.reset();
		m_cache.Clear();

		m_stats = Statistics();
		Logger::Info("All shader programs cleared");
	}

	ShaderControllerImpl::Statistics ShaderControllerImpl::GetStats() const {
		return m_stats;
	}

	void ShaderControllerImpl::PrintDebugInfo() const {
		Logger::Info("=== ShaderControllerImpl Debug Info ===");
		Logger::Info("Total programs: " + std::to_string(m_stats.totalPrograms));
		Logger::Info("Compiled shaders: " + std::to_string(m_stats.compiledShaders));
		Logger::Info("Cache hits: " + std::to_string(m_stats.cacheHits));
		Logger::Info("Cache misses: " + std::to_string(m_stats.cacheMisses));
		Logger::Info("Loaded programs:");

		for (const auto& pair : m_programs) {
			Logger::Info("  - " + pair.first +
				" (ID: " + std::to_string(pair.second->GetID()) + ")");
		}
	}

	void ShaderControllerImpl::WatchShaderFiles(bool enable) {
		m_watchFiles = enable;
		Logger::Info("Shader file watching " +
			std::string(enable ? "enabled" : "disabled"));
	}

	void ShaderControllerImpl::CheckForUpdates() {
		if (!m_watchFiles) return;
		// Заглушка для hot-reload
	}

	// ==================== PRIVATE METHODS ====================

	void ShaderControllerImpl::CleanupShaders(const std::vector<GLuint>& shaderIds) {
		for (GLuint shaderId : shaderIds) {
			if (shaderId != 0) {
				glDeleteShader(shaderId);
			}
		}
	}

	std::string ShaderControllerImpl::GenerateCacheKey(
		const std::string& name,
		const std::string& vertexSource,
		const std::string& fragmentSource,
		const std::string& geometrySource) {

		return name + "_" +
			std::to_string(std::hash<std::string>{}(vertexSource)) + "_" +
			std::to_string(std::hash<std::string>{}(fragmentSource)) + "_" +
			(geometrySource.empty() ? "" :
				std::to_string(std::hash<std::string>{}(geometrySource)));
	}

	std::string ShaderControllerImpl::GetBuiltinName(Builtin type) {
		switch (type) {
		case Builtin::BasicColor: return "Builtin_BasicColor";
		case Builtin::BasicTexture: return "Builtin_BasicTexture";
		case Builtin::Skybox: return "Builtin_Skybox";
		case Builtin::Unlit: return "Builtin_Unlit";
		case Builtin::Wireframe: return "Builtin_Wireframe";
		default: return "Builtin_Unknown";
		}
	}

	void ShaderControllerImpl::GetBuiltinSources(Builtin type,
		std::string& outVertex,
		std::string& outFragment) {
		switch (type) {
		case Builtin::BasicColor:
			outVertex = m_loader.GetBuiltinSource("BasicColor.vert");
			outFragment = m_loader.GetBuiltinSource("BasicColor.frag");
			break;

		case Builtin::BasicTexture:
			outVertex = m_loader.GetBuiltinSource("BasicTexture.vert");
			outFragment = m_loader.GetBuiltinSource("BasicTexture.frag");
			break;

		case Builtin::Skybox:
			outVertex = m_loader.GetBuiltinSource("Skybox.vert");
			outFragment = m_loader.GetBuiltinSource("Skybox.frag");
			break;

		case Builtin::Unlit:
			outVertex = m_loader.GetBuiltinSource("Unlit.vert");
			outFragment = m_loader.GetBuiltinSource("Unlit.frag");
			break;

		case Builtin::Wireframe:
			outVertex = m_loader.GetBuiltinSource("Wireframe.vert");
			outFragment = m_loader.GetBuiltinSource("Wireframe.frag");
			break;

		default:
			outVertex = m_loader.GetBuiltinSource("BasicColor.vert");
			outFragment = m_loader.GetBuiltinSource("BasicColor.frag");
			break;
		}
	}

} // namespace ogle