#pragma once

#include <string>

extern "C" {
#include <duktape.h>
}

class IWorldAccess;
namespace OGLE { class World; }

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    bool Initialize(IWorldAccess& worldAccess);
    void Shutdown();

    bool ExecuteFile(const std::string& scriptPath);
    void Update(float deltaTime);

private:
    OGLE::World* GetWorld();
    const OGLE::World* GetWorld() const;

    static ScriptManager* GetInstance(duk_context* context);
    static duk_ret_t JsLog(duk_context* context);
    static duk_ret_t JsClearWorld(duk_context* context);
    static duk_ret_t JsSpawnCube(duk_context* context);
    static duk_ret_t JsSetPosition(duk_context* context);
    static duk_ret_t JsSetRotation(duk_context* context);
    static duk_ret_t JsSetScale(duk_context* context);
    static duk_ret_t JsSetTexture(duk_context* context);
    static duk_ret_t JsEntityExists(duk_context* context);
    static duk_ret_t JsSaveWorld(duk_context* context);
    static duk_ret_t JsLoadWorld(duk_context* context);
    static duk_ret_t JsCreateEmpty(duk_context* context);
    static duk_ret_t JsCreateModel(duk_context* context);
    static duk_ret_t JsCreateDirectionalLight(duk_context* context);
    static duk_ret_t JsCreatePointLight(duk_context* context);
    static duk_ret_t JsDestroyEntity(duk_context* context);
    static duk_ret_t JsFindEntityByName(duk_context* context);
    static duk_ret_t JsGetAllEntities(duk_context* context);
    static duk_ret_t JsGetName(duk_context* context);
    static duk_ret_t JsSetName(duk_context* context);
    static duk_ret_t JsGetPosition(duk_context* context);
    static duk_ret_t JsGetRotation(duk_context* context);
    static duk_ret_t JsGetScale(duk_context* context);
    static duk_ret_t JsGetEnabled(duk_context* context);
    static duk_ret_t JsSetEnabled(duk_context* context);
    static duk_ret_t JsGetVisible(duk_context* context);
    static duk_ret_t JsSetVisible(duk_context* context);
    static duk_ret_t JsGetEntityKind(duk_context* context);
    static duk_ret_t JsGetEntitiesByKind(duk_context* context);
    static duk_ret_t JsGetEntityCount(duk_context* context);
    static duk_ret_t JsGetTexture(duk_context* context);
    static duk_ret_t JsGetMaterialBaseColor(duk_context* context);
    static duk_ret_t JsSetMaterialBaseColor(duk_context* context);
    static duk_ret_t JsGetMaterialEmissiveColor(duk_context* context);
    static duk_ret_t JsSetMaterialEmissiveColor(duk_context* context);
    static duk_ret_t JsGetMaterialUvTiling(duk_context* context);
    static duk_ret_t JsSetMaterialUvTiling(duk_context* context);
    static duk_ret_t JsGetMaterialUvOffset(duk_context* context);
    static duk_ret_t JsSetMaterialUvOffset(duk_context* context);
    static duk_ret_t JsGetMaterialRoughness(duk_context* context);
    static duk_ret_t JsSetMaterialRoughness(duk_context* context);
    static duk_ret_t JsGetMaterialMetallic(duk_context* context);
    static duk_ret_t JsSetMaterialMetallic(duk_context* context);
    static duk_ret_t JsGetMaterialAlphaCutoff(duk_context* context);
    static duk_ret_t JsSetMaterialAlphaCutoff(duk_context* context);
    static duk_ret_t JsGetEmissiveTexture(duk_context* context);
    static duk_ret_t JsSetEmissiveTexture(duk_context* context);
    static duk_ret_t JsGetLightColor(duk_context* context);
    static duk_ret_t JsSetLightColor(duk_context* context);
    static duk_ret_t JsGetLightIntensity(duk_context* context);
    static duk_ret_t JsSetLightIntensity(duk_context* context);
    static duk_ret_t JsGetLightRange(duk_context* context);
    static duk_ret_t JsSetLightRange(duk_context* context);
    static duk_ret_t JsGetLightCastShadows(duk_context* context);
    static duk_ret_t JsSetLightCastShadows(duk_context* context);
    static duk_ret_t JsGetLightPrimary(duk_context* context);
    static duk_ret_t JsSetLightPrimary(duk_context* context);
    static duk_ret_t JsGetLightType(duk_context* context);
    static duk_ret_t JsSetLightType(duk_context* context);

    void RegisterBindings();
    bool InstallApiBootstrap();
    bool CallGlobalFunction(const char* functionName);
    bool CallGlobalFunction(const char* functionName, float argument);
    std::string ResolveScriptPath(const std::string& scriptPath) const;

    duk_context* m_context = nullptr;
    IWorldAccess* m_worldAccess = nullptr;
};
