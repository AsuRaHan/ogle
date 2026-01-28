// src/render/ShaderController.cpp
#include "ShaderController.h"

namespace fs = std::filesystem;

namespace ogle {

    ShaderController& ShaderController::Get() {
        static ShaderController instance;
        return instance;
    }

    ShaderController::ShaderController() {
        Logger::Info("ShaderController initialized");
        PreloadBuiltinShaders();
    }

    ShaderController::~ShaderController() {
        Clear();
        Logger::Info("ShaderController shutdown");
    }

    std::shared_ptr<ShaderProgram> ShaderController::CreateProgram(
        const std::string& name,
        const std::string& vertexSource,
        const std::string& fragmentSource,
        const std::string& geometrySource) {

        auto it = m_programs.find(name);
        if (it != m_programs.end()) {
            Logger::Warning("Shader program already exists: " + name);
            return it->second;
        }

        std::string cacheKey = GenerateCacheKey(name, vertexSource, fragmentSource, geometrySource);
        GLuint cachedId = m_cache.GetProgram(cacheKey);
        if (cachedId != 0) {
            auto program = ShaderProgram::Create(cachedId, name);
            m_programs[name] = program;
            m_stats.cacheHits++;
            Logger::Info("Shader program loaded from cache: " + name);
            return program;
        }

        m_stats.cacheMisses++;

        std::vector<GLuint> shaderIds;

        auto vertexResult = m_compiler.Compile(GL_VERTEX_SHADER, vertexSource, name + "_vertex");
        if (!vertexResult.success) { Logger::Error("Failed to compile vertex shader for: " + name); return nullptr; }
        shaderIds.push_back(vertexResult.shaderId); m_stats.compiledShaders++;

        auto fragmentResult = m_compiler.Compile(GL_FRAGMENT_SHADER, fragmentSource, name + "_fragment");
        if (!fragmentResult.success) { CleanupShaders(shaderIds); Logger::Error("Failed to compile fragment shader for: " + name); return nullptr; }
        shaderIds.push_back(fragmentResult.shaderId); m_stats.compiledShaders++;

        if (!geometrySource.empty()) {
            auto geometryResult = m_compiler.Compile(GL_GEOMETRY_SHADER, geometrySource, name + "_geometry");
            if (!geometryResult.success) { CleanupShaders(shaderIds); Logger::Error("Failed to compile geometry shader for: " + name); return nullptr; }
            shaderIds.push_back(geometryResult.shaderId); m_stats.compiledShaders++;
        }

        auto linkResult = m_linker.Link(shaderIds, name);
        if (!linkResult.success) { CleanupShaders(shaderIds); Logger::Error("Failed to link shader program: " + name); return nullptr; }

        CleanupShaders(shaderIds);

        auto program = ShaderProgram::Create(linkResult.programId, name);
        m_programs[name] = program;
        m_cache.StoreProgram(cacheKey, linkResult.programId);

        if (!vertexSource.empty() && !fragmentSource.empty()) {
            ShaderProgramInfo info;
            info.program = program;
            info.vertexSource = vertexSource;
            info.fragmentSource = fragmentSource;
            info.geometrySource = geometrySource;
            info.lastModified = fs::file_time_type::clock::now();
            m_programInfos[name] = info;
        }

        m_stats.totalPrograms++;
        Logger::Info("Shader program created: " + name + " (ID: " + std::to_string(linkResult.programId) + ")");
        return program;
    }

    std::shared_ptr<ShaderProgram> ShaderController::CreateProgramFromFiles(
        const std::string& name,
        const std::string& vertexPath,
        const std::string& fragmentPath,
        const std::string& geometryPath) {

        auto it = m_programs.find(name);
        if (it != m_programs.end()) { Logger::Warning("Shader program already exists: " + name); return it->second; }

        std::string vertexSource = m_loader.LoadFromFile(vertexPath);
        std::string fragmentSource = m_loader.LoadFromFile(fragmentPath);
        if (vertexSource.empty() || fragmentSource.empty()) { Logger::Error("Failed to load shader files for: " + name); return nullptr; }

        vertexSource = m_loader.Preprocess(vertexSource, vertexPath);
        fragmentSource = m_loader.Preprocess(fragmentSource, fragmentPath);

        std::string geometrySource;
        if (!geometryPath.empty()) {
            geometrySource = m_loader.LoadFromFile(geometryPath);
            if (!geometrySource.empty()) geometrySource = m_loader.Preprocess(geometrySource, geometryPath);
        }

        auto program = CreateProgram(name, vertexSource, fragmentSource, geometrySource);
        if (!program) return nullptr;

        auto infoIt = m_programInfos.find(name);
        if (infoIt != m_programInfos.end()) {
            infoIt->second.vertexPath = vertexPath;
            infoIt->second.fragmentPath = fragmentPath;
            infoIt->second.geometryPath = geometryPath;

            auto getLastWriteTime = [](const std::string& path) {
                try { return fs::last_write_time(path); } catch (...) { return fs::file_time_type::min(); }
            };

            auto lastModified = getLastWriteTime(vertexPath);
            if (!fragmentPath.empty()) { auto ft = getLastWriteTime(fragmentPath); if (ft > lastModified) lastModified = ft; }
            if (!geometryPath.empty()) { auto ft = getLastWriteTime(geometryPath); if (ft > lastModified) lastModified = ft; }
            infoIt->second.lastModified = lastModified;
        }

        Logger::Debug("Shader program loaded from files: " + name);
        return program;
    }

    void ShaderController::WatchShaderFiles(bool enable) {
        m_watchEnabled = enable;
        Logger::Info("Shader file watching " + std::string(enable ? "enabled" : "disabled"));
    }

    void ShaderController::CheckForUpdates() {
        if (!m_watchEnabled) return;

        bool anyChanged = false;

        for (auto& pair : m_programInfos) {
            const std::string& name = pair.first;
            ShaderProgramInfo& info = pair.second;
            if (info.vertexPath.empty() && info.fragmentPath.empty()) continue;

            bool needsRecompile = false;
            std::string newVertexSource = info.vertexSource;
            std::string newFragmentSource = info.fragmentSource;
            std::string newGeometrySource = info.geometrySource;

            if (!info.vertexPath.empty()) {
                if (m_loader.HasFileChanged(info.vertexPath)) {
                    std::string source = m_loader.LoadFromFile(info.vertexPath);
                    if (!source.empty()) { source = m_loader.Preprocess(source, info.vertexPath); if (source != info.vertexSource) { newVertexSource = source; needsRecompile = true; Logger::Info("Vertex shader updated: " + info.vertexPath); } }
                }
            }

            if (!info.fragmentPath.empty()) {
                if (m_loader.HasFileChanged(info.fragmentPath)) {
                    std::string source = m_loader.LoadFromFile(info.fragmentPath);
                    if (!source.empty()) { source = m_loader.Preprocess(source, info.fragmentPath); if (source != info.fragmentSource) { newFragmentSource = source; needsRecompile = true; Logger::Info("Fragment shader updated: " + info.fragmentPath); } }
                }
            }

            if (!info.geometryPath.empty() && !info.geometrySource.empty()) {
                if (m_loader.HasFileChanged(info.geometryPath)) {
                    std::string source = m_loader.LoadFromFile(info.geometryPath);
                    if (!source.empty()) { source = m_loader.Preprocess(source, info.geometryPath); if (source != info.geometrySource) { newGeometrySource = source; needsRecompile = true; Logger::Info("Geometry shader updated: " + info.geometryPath); } }
                }
            }

            if (needsRecompile) {
                std::vector<GLuint> shaderIds;
                bool compileSuccess = true;

                auto vertexResult = m_compiler.Compile(GL_VERTEX_SHADER, newVertexSource, name + "_vertex_reload");
                if (!vertexResult.success) { Logger::Error("Hot-reload failed: vertex shader compilation error"); compileSuccess = false; } else shaderIds.push_back(vertexResult.shaderId);

                auto fragmentResult = m_compiler.Compile(GL_FRAGMENT_SHADER, newFragmentSource, name + "_fragment_reload");
                if (!fragmentResult.success) { Logger::Error("Hot-reload failed: fragment shader compilation error"); compileSuccess = false; } else shaderIds.push_back(fragmentResult.shaderId);

                if (!newGeometrySource.empty()) {
                    auto geometryResult = m_compiler.Compile(GL_GEOMETRY_SHADER, newGeometrySource, name + "_geometry_reload");
                    if (!geometryResult.success) { Logger::Error("Hot-reload failed: geometry shader compilation error"); compileSuccess = false; } else shaderIds.push_back(geometryResult.shaderId);
                }

                if (compileSuccess) {
                    auto linkResult = m_linker.Link(shaderIds, name + "_reload");
                    CleanupShaders(shaderIds);

                    if (linkResult.success) {
                        auto newProgram = ShaderProgram::Create(linkResult.programId, name);
                        info.program = newProgram;
                        info.vertexSource = newVertexSource;
                        info.fragmentSource = newFragmentSource;
                        info.geometrySource = newGeometrySource;

                        try {
                            auto lastModified = fs::last_write_time(info.vertexPath);
                            if (!info.fragmentPath.empty()) { auto ft = fs::last_write_time(info.fragmentPath); if (ft > lastModified) lastModified = ft; }
                            if (!info.geometryPath.empty()) { auto ft = fs::last_write_time(info.geometryPath); if (ft > lastModified) lastModified = ft; }
                            info.lastModified = lastModified;
                        } catch (...) { info.lastModified = fs::file_time_type::clock::now(); }

                        m_programs[name] = newProgram;

                        std::string cacheKey = GenerateCacheKey(name, newVertexSource, newFragmentSource, newGeometrySource);
                        m_cache.StoreProgram(cacheKey, linkResult.programId);

                        if (m_currentProgram && m_currentProgram->GetName() == name) {
                            UseProgram(name);
                        }

                        Logger::Success("Shader hot-reloaded: " + name);
                        anyChanged = true;
                    } else {
                        Logger::Error("Hot-reload failed: linking error"); CleanupShaders(shaderIds);
                    }
                } else {
                    CleanupShaders(shaderIds);
                }
            }
        }

        if (anyChanged) Logger::Debug("Shader hot-reload completed");
    }

    std::shared_ptr<ShaderProgram> ShaderController::GetProgram(const std::string& name) {
        auto it = m_programs.find(name);
        return it != m_programs.end() ? it->second : nullptr;
    }

    bool ShaderController::HasProgram(const std::string& name) const { return m_programs.find(name) != m_programs.end(); }

    void ShaderController::UseProgram(const std::string& name) {
        auto program = GetProgram(name);
        if (program) { program->Bind(); m_currentProgram = program; }
    }

    void ShaderController::UseProgram(std::shared_ptr<ShaderProgram> program) { if (program) { program->Bind(); m_currentProgram = program; } }

    std::shared_ptr<ShaderProgram> ShaderController::GetCurrentProgram() const { return m_currentProgram; }

    std::shared_ptr<ShaderProgram> ShaderController::GetBuiltin(Builtin type) {
        std::string builtinName = GetBuiltinName(type);
        auto it = m_programs.find(builtinName);
        if (it != m_programs.end()) return it->second;
        std::string vertexSource, fragmentSource; GetBuiltinSources(type, vertexSource, fragmentSource);
        auto program = CreateProgram(builtinName, vertexSource, fragmentSource, "");
        if (program) Logger::Info("Builtin shader created: " + builtinName);
        return program;
    }

    void ShaderController::PreloadBuiltinShaders() {
        Logger::Info("Preloading builtin shaders...");
        GetBuiltin(Builtin::BasicColor);
        GetBuiltin(Builtin::BasicTexture);
        GetBuiltin(Builtin::Skybox);
        GetBuiltin(Builtin::Unlit);
        GetBuiltin(Builtin::Wireframe);
        Logger::Info("Builtin shaders preloaded");
    }

    void ShaderController::RemoveProgram(const std::string& name) {
        auto it = m_programs.find(name);
        if (it != m_programs.end()) {
            if (m_currentProgram == it->second) m_currentProgram.reset();
            m_programs.erase(it);
            Logger::Info("Shader program removed: " + name);
            m_stats.totalPrograms--;
        }
        m_programInfos.erase(name);
    }

    void ShaderController::Clear() {
        m_programs.clear();
        m_programInfos.clear();
        m_currentProgram.reset();
        m_cache.Clear();
        m_stats = Statistics();
        Logger::Info("All shader programs cleared");
    }

    void ShaderController::CleanupShaders(const std::vector<GLuint>& shaderIds) {
        for (GLuint shaderId : shaderIds) if (shaderId != 0) glDeleteShader(shaderId);
    }

    std::string ShaderController::GenerateCacheKey(const std::string& name,
        const std::string& vertexSource,
        const std::string& fragmentSource,
        const std::string& geometrySource) {
        return name + "_" + std::to_string(std::hash<std::string>{}(vertexSource)) + "_" + std::to_string(std::hash<std::string>{}(fragmentSource)) + "_" + (geometrySource.empty() ? "" : std::to_string(std::hash<std::string>{}(geometrySource)));
    }

    std::string ShaderController::GetBuiltinName(Builtin type) {
        switch (type) {
        case Builtin::BasicColor: return "Builtin_BasicColor";
        case Builtin::BasicTexture: return "Builtin_BasicTexture";
        case Builtin::Skybox: return "Builtin_Skybox";
        case Builtin::Unlit: return "Builtin_Unlit";
        case Builtin::Wireframe: return "Builtin_Wireframe";
        default: return "Builtin_Unknown";
        }
    }

    void ShaderController::GetBuiltinSources(Builtin type, std::string& outVertex, std::string& outFragment) {
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

    ShaderController::Statistics ShaderController::GetStats() const { return m_stats; }

    void ShaderController::PrintDebugInfo() const {
        Logger::Info("=== ShaderController Debug Info ===");
        Logger::Info("Total programs: " + std::to_string(m_stats.totalPrograms));
        Logger::Info("Compiled shaders: " + std::to_string(m_stats.compiledShaders));
        Logger::Info("Cache hits: " + std::to_string(m_stats.cacheHits));
        Logger::Info("Cache misses: " + std::to_string(m_stats.cacheMisses));
        Logger::Info("Watching enabled: " + std::string(m_watchEnabled ? "Yes" : "No"));
        Logger::Info("Programs with hot-reload info: " + std::to_string(m_programInfos.size()));
        Logger::Info("Loaded programs:");
        for (const auto& pair : m_programs) Logger::Info("  - " + pair.first + " (ID: " + std::to_string(pair.second->GetID()) + ")");
    }

} // namespace ogle
