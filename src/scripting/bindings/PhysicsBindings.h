#pragma once

#include "scripting/bindings/Common.h"

namespace OGLE { namespace ScriptBindings {

void RegisterPhysicsBindings(duk_context* ctx, const NativeBinder& bindNative);

}}