#include "scripting/bindings/MaterialBindings.h"

namespace OGLE { namespace ScriptBindings {

Material* GetMutableMaterial(World& world, Entity entity)
{
    if (auto* matComp = world.GetMaterial(entity)) {
        return &matComp->material;
    }
    if (auto* model = world.GetModel(entity)) {
        return &model->GetMaterial();
    }
    return nullptr;
}

duk_ret_t JsSetBaseColor(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* mat = GetMutableMaterial(*world, entity)) {
            if (duk_is_object(context, 1)) {
                glm::vec3 color;
                duk_get_prop_string(context, 1, "x"); color.x = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "y"); color.y = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "z"); color.z = (float)duk_get_number(context, -1); duk_pop(context);
                mat->SetBaseColor(color);
            }
        }
    }
    return 0;
}

duk_ret_t JsSetEmissiveColor(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* mat = GetMutableMaterial(*world, entity)) {
            if (duk_is_object(context, 1)) {
                glm::vec3 color;
                duk_get_prop_string(context, 1, "x"); color.x = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "y"); color.y = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "z"); color.z = (float)duk_get_number(context, -1); duk_pop(context);
                mat->SetEmissiveColor(color);
            }
        }
    }
    return 0;
}

duk_ret_t JsSetRoughness(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* mat = GetMutableMaterial(*world, entity)) {
            mat->SetRoughness((float)duk_require_number(context, 1));
        }
    }
    return 0;
}

duk_ret_t JsSetMetallic(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* mat = GetMutableMaterial(*world, entity)) {
            mat->SetMetallic((float)duk_require_number(context, 1));
        }
    }
    return 0;
}

duk_ret_t JsSetReflectivity(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* mat = GetMutableMaterial(*world, entity)) {
            // mat->SetReflectivity((float)duk_require_number(context, 1));
        }
    }
    return 0;
}

void RegisterMaterialBindings(duk_context* ctx, const NativeBinder& bindNative)
{
    bindNative("setBaseColor", JsSetBaseColor, 2);
    bindNative("setEmissiveColor", JsSetEmissiveColor, 2);
    bindNative("setRoughness", JsSetRoughness, 2);
    bindNative("setMetallic", JsSetMetallic, 2);
    bindNative("setReflectivity", JsSetReflectivity, 2);
}

}}