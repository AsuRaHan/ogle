#include "scripting/bindings/LightBindings.h"

namespace OGLE { namespace ScriptBindings {

duk_ret_t JsSetIntensity(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* light = world->GetComponent<LightComponent>(entity)) {
            light->intensity = (float)duk_require_number(context, 1);
        }
    }
    return 0;
}

void RegisterLightBindings(duk_context* ctx, const NativeBinder& bindNative)
{
    bindNative("setIntensity", JsSetIntensity, 2);
}

}}