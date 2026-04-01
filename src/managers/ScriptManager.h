#pragma once

#include <string>

extern "C" {
#include <duktape.h>
}

class WorldManager;

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    bool Initialize(WorldManager& worldManager);
    void Shutdown();

    bool ExecuteFile(const std::string& scriptPath);
    void Update(float deltaTime);

private:
    static ScriptManager* GetInstance(duk_context* context);
    static duk_ret_t JsLog(duk_context* context);
    static duk_ret_t JsClearWorld(duk_context* context);
    static duk_ret_t JsSpawnCube(duk_context* context);
    static duk_ret_t JsSetPosition(duk_context* context);
    static duk_ret_t JsSetRotation(duk_context* context);
    static duk_ret_t JsSetScale(duk_context* context);
    static duk_ret_t JsSetTexture(duk_context* context);
    static duk_ret_t JsEntityExists(duk_context* context);

    void RegisterBindings();
    bool CallGlobalFunction(const char* functionName);
    bool CallGlobalFunction(const char* functionName, float argument);
    std::string ResolveScriptPath(const std::string& scriptPath) const;

    duk_context* m_context = nullptr;
    WorldManager* m_worldManager = nullptr;
};
