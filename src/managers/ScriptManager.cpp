#include "managers/ScriptManager.h"

#include "Logger.h"
#include "managers/WorldManager.h"
#include "world/WorldComponents.h"

#include <duktape.h>
#include <glm/vec3.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <windows.h>

namespace
{
    constexpr const char* kScriptManagerGlobal = "__ogleScriptManager";

    OGLE::Entity ToEntity(duk_uint_t rawEntity)
    {
        return static_cast<OGLE::Entity>(rawEntity);
    }
}

ScriptManager::ScriptManager() = default;

ScriptManager::~ScriptManager()
{
    Shutdown();
}

bool ScriptManager::Initialize(WorldManager& worldManager)
{
    Shutdown();

    m_context = duk_create_heap_default();
    if (!m_context) {
        LOG_ERROR("Failed to create Duktape heap");
        return false;
    }

    m_worldManager = &worldManager;

    duk_push_pointer(m_context, this);
    duk_put_global_string(m_context, kScriptManagerGlobal);

    RegisterBindings();

    LOG_INFO("ScriptManager initialized");
    return true;
}

void ScriptManager::Shutdown()
{
    if (m_context) {
        duk_destroy_heap(m_context);
        m_context = nullptr;
    }

    m_worldManager = nullptr;
}

bool ScriptManager::ExecuteFile(const std::string& scriptPath)
{
    if (!m_context) {
        LOG_ERROR("ScriptManager is not initialized");
        return false;
    }

    const std::string resolvedPath = ResolveScriptPath(scriptPath);
    if (resolvedPath.empty()) {
        LOG_ERROR("Script file not found: " + scriptPath);
        return false;
    }

    std::ifstream input(resolvedPath, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        LOG_ERROR("Unable to open script file: " + resolvedPath);
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string source = buffer.str();

    if (duk_peval_lstring(m_context, source.c_str(), source.size()) != 0) {
        LOG_ERROR("Script evaluation failed: " + std::string(duk_safe_to_string(m_context, -1)));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);

    if (!CallGlobalFunction("onStart")) {
        LOG_WARN("Script loaded, but onStart failed");
        return false;
    }

    LOG_INFO("Executed script: " + resolvedPath);
    return true;
}

void ScriptManager::Update(float deltaTime)
{
    if (!m_context) {
        return;
    }

    CallGlobalFunction("onUpdate", deltaTime);
}

ScriptManager* ScriptManager::GetInstance(duk_context* context)
{
    duk_get_global_string(context, kScriptManagerGlobal);
    ScriptManager* manager = static_cast<ScriptManager*>(duk_get_pointer(context, -1));
    duk_pop(context);
    return manager;
}

duk_ret_t ScriptManager::JsLog(duk_context* context)
{
    const char* message = duk_safe_to_string(context, 0);
    LOG_INFO(std::string("[JS] ") + (message ? message : ""));
    return 0;
}

duk_ret_t ScriptManager::JsClearWorld(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        return DUK_RET_ERROR;
    }

    manager->m_worldManager->ClearWorld();
    return 0;
}

duk_ret_t ScriptManager::JsSpawnCube(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        return DUK_RET_ERROR;
    }

    const char* name = duk_opt_string(context, 0, "ScriptCube");
    const float x = static_cast<float>(duk_opt_number(context, 1, 0.0));
    const float y = static_cast<float>(duk_opt_number(context, 2, 0.0));
    const float z = static_cast<float>(duk_opt_number(context, 3, 0.0));
    const float sx = static_cast<float>(duk_opt_number(context, 4, 1.0));
    const float sy = static_cast<float>(duk_opt_number(context, 5, 1.0));
    const float sz = static_cast<float>(duk_opt_number(context, 6, 1.0));

    const OGLE::Entity entity = manager->m_worldManager->CreateCube(
        name ? name : "ScriptCube",
        glm::vec3(x, y, z),
        glm::vec3(sx, sy, sz));

    duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsSetPosition(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const glm::vec3 position(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));

    duk_push_boolean(context, manager->m_worldManager->SetEntityPosition(entity, position));
    return 1;
}

duk_ret_t ScriptManager::JsSetRotation(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const glm::vec3 rotation(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));

    duk_push_boolean(context, manager->m_worldManager->SetEntityRotation(entity, rotation));
    return 1;
}

duk_ret_t ScriptManager::JsSetScale(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const glm::vec3 scale(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));

    duk_push_boolean(context, manager->m_worldManager->SetEntityScale(entity, scale));
    return 1;
}

duk_ret_t ScriptManager::JsEntityExists(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    duk_push_boolean(context, manager->m_worldManager->IsEntityValid(entity));
    return 1;
}

duk_ret_t ScriptManager::JsSetTexture(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldManager) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const char* texturePath = duk_safe_to_string(context, 1);
    duk_push_boolean(context, manager->m_worldManager->SetEntityDiffuseTexture(
        entity,
        texturePath ? texturePath : ""));
    return 1;
}

void ScriptManager::RegisterBindings()
{
    duk_push_c_function(m_context, JsLog, 1);
    duk_put_global_string(m_context, "log");

    duk_push_c_function(m_context, JsClearWorld, 0);
    duk_put_global_string(m_context, "clearWorld");

    duk_push_c_function(m_context, JsSpawnCube, DUK_VARARGS);
    duk_put_global_string(m_context, "spawnCube");

    duk_push_c_function(m_context, JsSetPosition, 4);
    duk_put_global_string(m_context, "setPosition");

    duk_push_c_function(m_context, JsSetRotation, 4);
    duk_put_global_string(m_context, "setRotation");

    duk_push_c_function(m_context, JsSetScale, 4);
    duk_put_global_string(m_context, "setScale");

    duk_push_c_function(m_context, JsSetTexture, 2);
    duk_put_global_string(m_context, "setTexture");

    duk_push_c_function(m_context, JsEntityExists, 1);
    duk_put_global_string(m_context, "entityExists");
}

bool ScriptManager::CallGlobalFunction(const char* functionName, float argument)
{
    duk_get_global_string(m_context, functionName);
    if (!duk_is_function(m_context, -1)) {
        duk_pop(m_context);
        return true;
    }

    duk_push_number(m_context, argument);
    if (duk_pcall(m_context, 1) != 0) {
        LOG_ERROR("Script function failed: " + std::string(functionName) +
            " -> " + duk_safe_to_string(m_context, -1));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);
    return true;
}

bool ScriptManager::CallGlobalFunction(const char* functionName)
{
    duk_get_global_string(m_context, functionName);
    if (!duk_is_function(m_context, -1)) {
        duk_pop(m_context);
        return true;
    }

    if (duk_pcall(m_context, 0) != 0) {
        LOG_ERROR("Script function failed: " + std::string(functionName) +
            " -> " + duk_safe_to_string(m_context, -1));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);
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
