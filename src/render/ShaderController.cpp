// src/render/ShaderController.cpp
#include "ShaderController.h"
#include "shader/ShaderControllerImpl.h"
#include "shader/ShaderProgram.h"

namespace ogle {

	ShaderController& ShaderController::Get() {
		static ShaderController instance;
		return instance;
	}

	ShaderController::ShaderController()
		: m_impl(std::make_unique<ShaderControllerImpl>()) {
	}

	ShaderController::~ShaderController() = default;

	std::shared_ptr<ShaderProgram> ShaderController::CreateProgram(
		const std::string& name,
		const std::string& vertexSource,
		const std::string& fragmentSource,
		const std::string& geometrySource) {

		return m_impl->CreateProgram(name, vertexSource, fragmentSource, geometrySource);
	}

	std::shared_ptr<ShaderProgram> ShaderController::CreateProgramFromFiles(
		const std::string& name,
		const std::string& vertexPath,
		const std::string& fragmentPath,
		const std::string& geometryPath) {

		return m_impl->CreateProgramFromFiles(name, vertexPath, fragmentPath, geometryPath);
	}

	std::shared_ptr<ShaderProgram> ShaderController::GetProgram(const std::string& name) {
		return m_impl->GetProgram(name);
	}

	bool ShaderController::HasProgram(const std::string& name) const {
		return m_impl->HasProgram(name);
	}

	void ShaderController::UseProgram(const std::string& name) {
		m_impl->UseProgram(name);
	}

	void ShaderController::UseProgram(std::shared_ptr<ShaderProgram> program) {
		m_impl->UseProgram(program);
	}

	std::shared_ptr<ShaderProgram> ShaderController::GetCurrentProgram() const {
		return m_impl->GetCurrentProgram();
	}

	std::shared_ptr<ShaderProgram> ShaderController::GetBuiltin(Builtin type) {
		return m_impl->GetBuiltin(static_cast<ShaderControllerImpl::Builtin>(type));
	}

	void ShaderController::PreloadBuiltinShaders() {
		m_impl->PreloadBuiltinShaders();
	}

	void ShaderController::RemoveProgram(const std::string& name) {
		m_impl->RemoveProgram(name);
	}

	void ShaderController::Clear() {
		m_impl->Clear();
	}

	ShaderController::Statistics ShaderController::GetStats() const {
		auto stats = m_impl->GetStats();
		return Statistics{ stats.totalPrograms, stats.compiledShaders,
						 stats.cacheHits, stats.cacheMisses };
	}

	void ShaderController::PrintDebugInfo() const {
		m_impl->PrintDebugInfo();
	}

	void ShaderController::WatchShaderFiles(bool enable) {
		m_impl->WatchShaderFiles(enable);
	}

	void ShaderController::CheckForUpdates() {
		m_impl->CheckForUpdates();
	}

} // namespace ogle