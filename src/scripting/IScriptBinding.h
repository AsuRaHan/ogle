#pragma once

class ScriptEngine;

class IScriptBinding
{
public:
    virtual ~IScriptBinding() = default;
    virtual void Register(ScriptEngine* engine) = 0;
};
