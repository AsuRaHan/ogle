#pragma once

class IWorldAccess;

namespace OGLE
{
    class ScriptEngine;

    namespace ScriptBindings
    {
        void Register(ScriptEngine& engine, IWorldAccess& worldAccess);
    }
}