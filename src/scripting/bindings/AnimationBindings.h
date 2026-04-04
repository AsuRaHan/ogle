#pragma once

#include <DuktapeCpp.h>

namespace OGLE
{
    class IWorldAccess;
    class PhysicsManager;
    namespace ScriptBindings
    {
        void RegisterAnimationBindings(duktape::api_object& ns, IWorldAccess& world, PhysicsManager& physics);
    }
}