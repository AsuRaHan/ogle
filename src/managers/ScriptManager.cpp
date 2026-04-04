#include "managers/ScriptManager.h"

#include "scripting/ScriptEngine.h"
#include "scripting/ScriptBindings.h"
#include <entt/entt.hpp>
#include "Logger.h"
#include "world/IWorldAccess.h"

#include <duktape.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
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
    
    duk_context* ctx = m_engine->GetContext();
    if (!ctx) {
        return;
    }

    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, "__collisionCallback");
    if (!duk_is_function(ctx, -1)) {
        duk_pop_2(ctx);
        return;
    }

    duk_push_uint(ctx, static_cast<duk_uint_t>(entt::to_integral(a)));
    duk_push_uint(ctx, static_cast<duk_uint_t>(entt::to_integral(b)));

    if (duk_pcall(ctx, 2) != 0) {
        LOG_ERROR("Collision callback script failed: " + std::string(duk_safe_to_string(ctx, -1)));
    }

    duk_pop_2(ctx); // pop result and stash
}

bool ScriptManager::CallGlobalFunction(const char* functionName, float argument)
{
    if (!m_engine) return false;
    duk_context* ctx = m_engine->GetContext();

    duk_get_global_string(ctx, functionName);
    if (!duk_is_function(ctx, -1)) {
        duk_pop(ctx);
        return true; // Not an error if function doesn't exist
    }

    duk_push_number(ctx, argument);
    if (duk_pcall(ctx, 1) != 0) {
        LOG_ERROR("Script function failed: " + std::string(functionName) +
            " -> " + duk_safe_to_string(ctx, -1));
        duk_pop(ctx);
        return false;
    }

    duk_pop(ctx);
    return true;
}

bool ScriptManager::CallGlobalFunction(const char* functionName)
{
    if (!m_engine) return false;
    duk_context* ctx = m_engine->GetContext();

    duk_get_global_string(ctx, functionName);
    if (!duk_is_function(ctx, -1)) {
        duk_pop(ctx);
        return true; // Not an error if function doesn't exist
    }

    if (duk_pcall(ctx, 0) != 0) {
        LOG_ERROR("Script function failed: " + std::string(functionName) +
            " -> " + duk_safe_to_string(ctx, -1));
        duk_pop(ctx);
        return false;
    }

    duk_pop(ctx);
    return true;
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
