#pragma once

#include "WorldComponents.h"

namespace OGLE {
    class World;
}

// Minimal abstraction for systems that need to read/write the active world
// without depending on higher-level managers.
class IWorldAccess
{
public:
    virtual ~IWorldAccess() = default;

    virtual OGLE::World& GetActiveWorld() = 0;
    virtual const OGLE::World& GetActiveWorld() const = 0;
    virtual bool IsEntityValid(OGLE::Entity entity) const = 0;
};

