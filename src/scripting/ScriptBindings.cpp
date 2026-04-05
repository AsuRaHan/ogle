#include "scripting/ScriptBindings.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptApi.h"
#include "scripting/Player.h"
#include "Logger.h"
#include <dukglue/dukglue.h>

namespace OGLE
{
    namespace ScriptBindings
    {
        void Register(ScriptEngine& engine, IWorldAccess& worldAccess, PhysicsManager& physicsManager)
        {
            duk_context* ctx = engine.GetContext();

            static WorldApi worldApi(&worldAccess);
            static EntityApi entityApi(&worldAccess);
            static PhysicsApi physicsApi(&physicsManager, &worldAccess);
            static InputApi inputApi;
            static LogApi logApi;

            dukglue_register_constructor<OGLE::Player, int>(ctx, "Player");
            dukglue_register_method<OGLE::Player>(ctx, &OGLE::Player::GetHP, "getHP");
            dukglue_register_method<OGLE::Player>(ctx, &OGLE::Player::TakeDamage, "takeDamage");

            dukglue_register_method(ctx, &WorldApi::clear, "clear");
            dukglue_register_method(ctx, &WorldApi::createCube, "createCube");
            dukglue_register_method(ctx, &WorldApi::createSphere, "createSphere");
            dukglue_register_method(ctx, &WorldApi::createDirectionalLight, "createDirectionalLight");
            dukglue_register_method(ctx, &WorldApi::createPointLight, "createPointLight");

            dukglue_register_method(ctx, &EntityApi::exists, "exists");
            dukglue_register_method(ctx, &EntityApi::getPosition, "getPosition");
            dukglue_register_method(ctx, &EntityApi::setPosition, "setPosition");
            dukglue_register_method(ctx, &EntityApi::getRotation, "getRotation");
            dukglue_register_method(ctx, &EntityApi::setRotation, "setRotation");
            dukglue_register_method(ctx, &EntityApi::getName, "getName");

            dukglue_register_method(ctx, &PhysicsApi::addBox, "addBox");

            dukglue_register_method(ctx, &InputApi::isKeyDown, "isKeyDown");
            dukglue_register_method(ctx, &InputApi::isKeyPressed, "isKeyPressed");
            dukglue_register_method(ctx, &InputApi::isKeyReleased, "isKeyReleased");
            dukglue_register_method(ctx, &InputApi::getMousePosition, "getMousePosition");
            dukglue_register_method(ctx, &InputApi::getMouseDelta, "getMouseDelta");
            dukglue_register_method(ctx, &InputApi::getMouseWheelDelta, "getMouseWheelDelta");
            dukglue_register_method(ctx, &InputApi::isMouseButtonDown, "isMouseButtonDown");
            dukglue_register_method(ctx, &InputApi::isMouseButtonPressed, "isMouseButtonPressed");
            dukglue_register_method(ctx, &InputApi::isMouseButtonReleased, "isMouseButtonReleased");
            dukglue_register_method(ctx, &InputApi::isGamepadConnected, "isGamepadConnected");
            dukglue_register_method(ctx, &InputApi::isGamepadButtonDown, "isGamepadButtonDown");
            dukglue_register_method(ctx, &InputApi::getGamepadAxis, "getGamepadAxis");
            dukglue_register_method(ctx, &InputApi::getAxis, "getAxis");
            dukglue_register_method(ctx, &InputApi::getAxisRaw, "getAxisRaw");
            dukglue_register_method(ctx, &InputApi::getCurrentContext, "getCurrentContext");

            dukglue_register_method(ctx, &LogApi::log, "log");

            dukglue_register_global(ctx, &worldApi, "world");
            dukglue_register_global(ctx, &entityApi, "entity");
            dukglue_register_global(ctx, &physicsApi, "physics");
            dukglue_register_global(ctx, &inputApi, "input");
            dukglue_register_global(ctx, &logApi, "log");

            duk_push_global_object(ctx);
            duk_push_object(ctx);
            duk_get_prop_string(ctx, -2, "world");
            duk_put_prop_string(ctx, -2, "world");
            duk_get_prop_string(ctx, -2, "entity");
            duk_put_prop_string(ctx, -2, "entity");
            duk_get_prop_string(ctx, -2, "physics");
            duk_put_prop_string(ctx, -2, "physics");
            duk_get_prop_string(ctx, -2, "input");
            duk_put_prop_string(ctx, -2, "input");
            duk_get_prop_string(ctx, -2, "log");
            duk_put_prop_string(ctx, -2, "log");
            duk_get_prop_string(ctx, -2, "Player");
            duk_put_prop_string(ctx, -2, "Player");
            duk_put_prop_string(ctx, -2, "ogle");
            duk_pop(ctx);
        }
    }
}
