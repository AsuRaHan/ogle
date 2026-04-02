#pragma once

#include <memory>
#include <string>

class IWorldAccess;

namespace OGLE
{
    class ScriptEngine;
}

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;

    bool Initialize(IWorldAccess& worldAccess, const std::string& apiBootstrapPath);
    void Shutdown();

    bool ExecuteFile(const std::string& scriptPath);
    void Update(float deltaTime);

private:
    bool CallGlobalFunction(const char* functionName);
    bool CallGlobalFunction(const char* functionName, float argument);

    std::string ResolveScriptPath(const std::string& scriptPath) const;

    std::unique_ptr<OGLE::ScriptEngine> m_engine;
    IWorldAccess* m_worldAccess = nullptr;
};