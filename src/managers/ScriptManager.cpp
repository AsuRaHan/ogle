#include "managers/ScriptManager.h"

#include "scripting/ScriptEngine.h"
#include "scripting/ScriptBindings.h"
#include <entt/entt.hpp>
#include "Logger.h"
#include "world/IWorldAccess.h"

#include <filesystem>
#include <windows.h>

ScriptManager::ScriptManager() = default;

ScriptManager::~ScriptManager()
{
    Shutdown();
}

bool ScriptManager::Initialize(IWorldAccess& worldAccess, PhysicsManager& physicsManager, const std::string& apiBootstrapPath)
{
    Shutdown();
    m_worldAccess = &worldAccess;

    try {
        m_engine = std::make_unique<OGLE::ScriptEngine>();
    } catch (const std::exception& e) {
        LOG_ERROR("ScriptEngine creation failed: " + std::string(e.what()));
        return false;
    }

    OGLE::ScriptBindings::Register(*m_engine, *m_worldAccess, physicsManager);

    LOG_INFO("ScriptManager initialized");
    return true;
}

void ScriptManager::Shutdown()
{
    m_engine.reset();
    m_worldAccess = nullptr;
}

bool ScriptManager::ExecuteFile(const std::string& scriptPath)
{
    if (!m_engine) {
        LOG_ERROR("ScriptManager is not initialized");
        return false;
    }

    const std::string resolvedPath = ResolveScriptPath(scriptPath);
    if (resolvedPath.empty()) {
        LOG_ERROR("Script file not found: " + scriptPath);
        return false;
    }

    if (!m_engine->ExecuteFile(resolvedPath)) {
        LOG_ERROR("Failed to execute script: " + resolvedPath);
        LOG_ERROR("Script execution details: " + m_engine->GetLastErrorDetails());
        return false;
    }

    if (!CallGlobalFunction("onStart")) {
        LOG_WARN("Script loaded, but onStart failed or not found in " + resolvedPath);
    }

    LOG_INFO("Executed script: " + resolvedPath);
    return true;
}

void ScriptManager::Update(float deltaTime)
{
    if (!m_engine) {
        return;
    }

    CallGlobalFunction("onUpdate", deltaTime);
}

void ScriptManager::NotifyCollision(OGLE::Entity a, OGLE::Entity b)
{
    if (!m_engine) return;

    unsigned int first = static_cast<unsigned int>(entt::to_integral(a));
    unsigned int second = static_cast<unsigned int>(entt::to_integral(b));
    m_engine->CallGlobalFunction("onCollision", first, second);
}

bool ScriptManager::CallGlobalFunction(const char* functionName, float argument)
{
    if (!m_engine) return false;
    return m_engine->CallGlobalFunction(functionName, argument);
}

bool ScriptManager::CallGlobalFunction(const char* functionName)
{
    if (!m_engine) return false;
    return m_engine->CallGlobalFunction(functionName);
}

std::string ScriptManager::ResolveScriptPath(const std::string& scriptPath) const
{
    namespace fs = std::filesystem;

    fs::path path(scriptPath);
    if (fs::exists(path)) {
        return path.string();
    }

    char moduleFileName[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, moduleFileName, MAX_PATH) != 0) {
        const fs::path executableDirectory = fs::path(moduleFileName).parent_path();
        const fs::path candidate = executableDirectory / path;
        if (fs::exists(candidate)) {
            return candidate.string();
        }
    }

    return {};
}
