#include "scripting/bindings/WorldBindings.h"
#include "Logger.h"

namespace OGLE { namespace ScriptBindings {

duk_ret_t JsClearWorld(duk_context* context)
{
    if (auto* world = GetWorld(context)) {
        world->Clear();
    }
    return 0;
}

duk_ret_t JsCreateCube(duk_context* context)
{
    auto* world = GetWorld(context);
    if (!world) return DUK_RET_ERROR;

    std::string name = "ScriptedCube";
    glm::vec3 pos(0.0f), scale(1.0f);

    if (duk_is_object(context, 0)) {
        duk_get_prop_string(context, 0, "name");
        if (duk_is_string(context, -1)) name = duk_get_string(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "position");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); pos.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); pos.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); pos.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);

        duk_get_prop_string(context, 0, "scale");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); scale.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); scale.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); scale.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);
    }

    auto model = PrimitiveFactory::CreatePrimitiveModel(PrimitiveType::Cube, "");
    const Entity entity = world->AddModel(std::move(model), name);
    world->SetTransform(entity, pos, glm::vec3(0.0f), scale);
    duk_push_uint(context, (duk_uint_t)entt::to_integral(entity));
    return 1;
}

duk_ret_t JsCreateSphere(duk_context* context)
{
    auto* world = GetWorld(context);
    if (!world) return DUK_RET_ERROR;

    std::string name = "ScriptedSphere";
    glm::vec3 pos(0.0f);
    float radius = 1.0f;

    if (duk_is_object(context, 0)) {
        duk_get_prop_string(context, 0, "name");
        if (duk_is_string(context, -1)) name = duk_get_string(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "position");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); pos.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); pos.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); pos.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);

        duk_get_prop_string(context, 0, "radius");
        if (duk_is_number(context, -1)) radius = (float)duk_get_number(context, -1);
        duk_pop(context);
    }

    auto model = PrimitiveFactory::CreatePrimitiveModel(PrimitiveType::Sphere, "");
    const Entity entity = world->AddModel(std::move(model), name);
    world->SetTransform(entity, pos, glm::vec3(0.0f), glm::vec3(radius * 2.0f));
    duk_push_uint(context, (duk_uint_t)entt::to_integral(entity));
    return 1;
}

duk_ret_t JsCreateDirectionalLight(duk_context* context)
{
    auto* world = GetWorld(context);
    if (!world) return DUK_RET_ERROR;

    std::string name = "DirectionalLight";
    glm::vec3 rotation(0.0f), color(1.0f);
    float intensity = 1.0f;
    bool castShadows = false, primary = false;

    if (duk_is_object(context, 0)) {
        duk_get_prop_string(context, 0, "name");
        if (duk_is_string(context, -1)) name = duk_get_string(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "rotation");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); rotation.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); rotation.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); rotation.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);

        duk_get_prop_string(context, 0, "color");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); color.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); color.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); color.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);

        duk_get_prop_string(context, 0, "intensity");
        if (duk_is_number(context, -1)) intensity = (float)duk_get_number(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "castShadows");
        if (duk_is_boolean(context, -1)) castShadows = duk_get_boolean(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "primary");
        if (duk_is_boolean(context, -1)) primary = duk_get_boolean(context, -1);
        duk_pop(context);
    }

    const Entity entity = world->CreateWorldObject(name, WorldObjectKind::Light).GetEntity();
    world->SetTransform(entity, glm::vec3(0.0f), rotation, glm::vec3(1.0f));
    auto& light = world->GetRegistry().emplace<LightComponent>(entity);
    light.type = LightType::Directional;
    light.color = color;
    light.intensity = intensity;
    light.castShadows = castShadows;
    light.primary = primary;
    duk_push_uint(context, (duk_uint_t)entt::to_integral(entity));
    return 1;
}

duk_ret_t JsCreatePointLight(duk_context* context)
{
    auto* world = GetWorld(context);
    if (!world) return DUK_RET_ERROR;

    std::string name = "PointLight";
    glm::vec3 position(0.0f), color(1.0f);
    float intensity = 1.0f, range = 10.0f;

    if (duk_is_object(context, 0)) {
        duk_get_prop_string(context, 0, "name");
        if (duk_is_string(context, -1)) name = duk_get_string(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "position");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); position.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); position.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); position.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);

        duk_get_prop_string(context, 0, "color");
        if (duk_is_object(context, -1)) {
            duk_get_prop_string(context, -1, "x"); color.x = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "y"); color.y = (float)duk_get_number(context, -1); duk_pop(context);
            duk_get_prop_string(context, -1, "z"); color.z = (float)duk_get_number(context, -1); duk_pop(context);
        }
        duk_pop(context);

        duk_get_prop_string(context, 0, "intensity");
        if (duk_is_number(context, -1)) intensity = (float)duk_get_number(context, -1);
        duk_pop(context);

        duk_get_prop_string(context, 0, "range");
        if (duk_is_number(context, -1)) range = (float)duk_get_number(context, -1);
        duk_pop(context);
    }

    const Entity entity = world->CreateWorldObject(name, WorldObjectKind::Light).GetEntity();
    world->SetTransform(entity, position, glm::vec3(0.0f), glm::vec3(1.0f));
    auto& light = world->GetRegistry().emplace<LightComponent>(entity);
    light.type = LightType::Point;
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    duk_push_uint(context, (duk_uint_t)entt::to_integral(entity));
    return 1;
}

void RegisterWorldBindings(duk_context* ctx, const NativeBinder& bindNative)
{
    bindNative("clear", JsClearWorld, 0);
    bindNative("createCube", JsCreateCube, 1);
    bindNative("createSphere", JsCreateSphere, 1);
    bindNative("createDirectionalLight", JsCreateDirectionalLight, 1);
    bindNative("createPointLight", JsCreatePointLight, 1);
}

}}