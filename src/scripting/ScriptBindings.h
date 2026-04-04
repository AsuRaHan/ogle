#pragma once

class IWorldAccess;
class PhysicsManager;

namespace OGLE {
    class ScriptEngine;

    namespace ScriptBindings
    {
        void Register(ScriptEngine& engine, IWorldAccess& worldAccess, PhysicsManager& physicsManager);
    }
}