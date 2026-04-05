#include "scripting/bindings/EntityBindings.h"

namespace OGLE { namespace ScriptBindings {

duk_ret_t JsEntityExists(duk_context* context)
{
    auto* worldAccess = GetWorldAccess(context);
    if (!worldAccess) {
        duk_push_false(context);
        return 1;
    }
    if (!duk_is_number(context, 0)) {
        duk_push_false(context);
        return 1;
    }
    const auto entity = ToEntity(duk_require_uint(context, 0));
    duk_push_boolean(context, worldAccess->IsEntityValid(entity));
    return 1;
}

duk_ret_t JsGetPosition(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        if (const auto* transform = world->GetComponent<TransformComponent>(ToEntity(duk_require_uint(context, 0)))) {
            PushVec3(context, transform->position);
            return 1;
        }
    }
    PushVec3(context, glm::vec3(0.0f));
    return 1;
}

duk_ret_t JsSetPosition(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* transform = world->GetComponent<TransformComponent>(entity)) {
            if (duk_is_object(context, 1)) {
                glm::vec3 pos;
                duk_get_prop_string(context, 1, "x"); pos.x = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "y"); pos.y = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "z"); pos.z = (float)duk_get_number(context, -1); duk_pop(context);
                world->SetTransform(entity, pos, transform->rotation, transform->scale);
            }
        }
    }
    return 0;
}

duk_ret_t JsGetRotation(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        if (const auto* transform = world->GetComponent<TransformComponent>(ToEntity(duk_require_uint(context, 0)))) {
            PushVec3(context, transform->rotation);
            return 1;
        }
    }
    PushVec3(context, glm::vec3(0.0f));
    return 1;
}

duk_ret_t JsSetRotation(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        const auto entity = ToEntity(duk_require_uint(context, 0));
        if (auto* transform = world->GetComponent<TransformComponent>(entity)) {
            if (duk_is_object(context, 1)) {
                glm::vec3 rot;
                duk_get_prop_string(context, 1, "x"); rot.x = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "y"); rot.y = (float)duk_get_number(context, -1); duk_pop(context);
                duk_get_prop_string(context, 1, "z"); rot.z = (float)duk_get_number(context, -1); duk_pop(context);
                world->SetTransform(entity, transform->position, rot, transform->scale);
            }
        }
    }
    return 0;
}

duk_ret_t JsGetName(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        if (const auto* name = world->GetComponent<NameComponent>(ToEntity(duk_require_uint(context, 0)))) {
            duk_push_string(context, name->value.c_str());
            return 1;
        }
    }
    duk_push_string(context, "");
    return 1;
}

void RegisterEntityBindings(duk_context* ctx, const NativeBinder& bindNative)
{
    bindNative("exists", JsEntityExists, 1);
    bindNative("getPosition", JsGetPosition, 1);
    bindNative("setPosition", JsSetPosition, 2);
    bindNative("getRotation", JsGetRotation, 1);
    bindNative("setRotation", JsSetRotation, 2);
    bindNative("getName", JsGetName, 1);
}

}}