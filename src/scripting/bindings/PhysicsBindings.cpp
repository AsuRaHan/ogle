#include "scripting/bindings/PhysicsBindings.h"

namespace OGLE { namespace ScriptBindings {

bool ParseBodyType(duk_context* context, const std::string& typeName, PhysicsBodyType& outType)
{
    if (typeName == "Static") { outType = PhysicsBodyType::Static; return true; }
    if (typeName == "Dynamic") { outType = PhysicsBodyType::Dynamic; return true; }
    if (typeName == "Kinematic") { outType = PhysicsBodyType::Kinematic; return true; }
    return false;
}

duk_ret_t JsAddBox(duk_context* context)
{
    auto* physics = GetPhysicsManager(context);
    if (!physics) return DUK_RET_ERROR;

    const auto entity = ToEntity(duk_require_uint(context, 0));
    glm::vec3 halfExtents;
    PhysicsBodyType bodyType = PhysicsBodyType::Dynamic;
    float mass = 1.0f;

    if (duk_is_object(context, 1)) {
        duk_get_prop_string(context, 1, "x"); halfExtents.x = (float)duk_get_number(context, -1); duk_pop(context);
        duk_get_prop_string(context, 1, "y"); halfExtents.y = (float)duk_get_number(context, -1); duk_pop(context);
        duk_get_prop_string(context, 1, "z"); halfExtents.z = (float)duk_get_number(context, -1); duk_pop(context);
    }
    if (duk_is_string(context, 2)) {
        ParseBodyType(context, duk_get_string(context, 2), bodyType);
    }
    if (duk_is_number(context, 3)) {
        mass = (float)duk_get_number(context, 3);
    }

    bool result = physics->AddBoxBody(entity, halfExtents, bodyType, mass);
    duk_push_boolean(context, result);
    return 1;
}

duk_ret_t JsSetCollisionCallback(duk_context* context)
{
    if (!duk_is_function(context, 0)) {
        return DUK_RET_TYPE_ERROR;
    }

    duk_push_global_stash(context);
    duk_dup(context, 0);
    duk_put_prop_string(context, -2, kCollisionCallback);
    duk_pop(context);

    return 0;
}

void RegisterPhysicsBindings(duk_context* ctx, const NativeBinder& bindNative)
{
    bindNative("addBox", JsAddBox, 4);
    bindNative("setCollisionCallback", JsSetCollisionCallback, 1);
}

}}