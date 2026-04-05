#include "scripting/ScriptBindings.h"
#include "scripting/ScriptEngine.h"
#include "scripting/Player.h"
#include "Logger.h"
#include "scripting/bindings/Common.h"
#include "scripting/bindings/LogBindings.h"
#include "scripting/bindings/WorldBindings.h"
#include "scripting/bindings/EntityBindings.h"
#include "scripting/bindings/MaterialBindings.h"
#include "scripting/bindings/LightBindings.h"
#include "scripting/bindings/PhysicsBindings.h"

namespace OGLE
{
    namespace ScriptBindings
    {
        static void JsLog(const std::string& message)
        {
            LOG_INFO(message);
        }

        void Register(ScriptEngine& engine, IWorldAccess& worldAccess, PhysicsManager& physicsManager)
        {
            duk_context* ctx = engine.GetContext();

            duk_push_global_stash(ctx);
            duk_push_pointer(ctx, &worldAccess);
            duk_put_prop_string(ctx, -2, kWorldAccessPtr);
            duk_push_pointer(ctx, &physicsManager);
            duk_put_prop_string(ctx, -2, kPhysicsManagerPtr);
            duk_pop(ctx);

            engine.RegisterFunction("log", &JsLog);
            engine.RegisterConstructor<Player, int>("Player");
            engine.RegisterMethod<Player>("getHP", &Player::GetHP);
            engine.RegisterMethod<Player>("takeDamage", &Player::TakeDamage);

            duk_push_global_object(ctx);
            duk_idx_t ogle_obj_idx = duk_push_object(ctx);

            auto create_module = [&](const char* name, auto registrationFunc) {
                duk_idx_t module_idx = duk_push_object(ctx);
                auto bindNative = [ctx, module_idx](const char* func_name, duk_c_function func, duk_idx_t nargs) {
                    duk_push_c_function(ctx, func, nargs);
                    duk_put_prop_string(ctx, module_idx, func_name);
                };
                registrationFunc(ctx, bindNative);
                duk_put_prop_string(ctx, ogle_obj_idx, name);
            };

            create_module("world", RegisterWorldBindings);
            create_module("entity", RegisterEntityBindings);
            create_module("material", RegisterMaterialBindings);
            create_module("light", RegisterLightBindings);
            create_module("physics", RegisterPhysicsBindings);
            RegisterLogBindings(ctx, [ctx, ogle_obj_idx](const char* func_name, duk_c_function func, duk_idx_t nargs) {
                duk_push_c_function(ctx, func, nargs);
                duk_put_prop_string(ctx, ogle_obj_idx, func_name);
            });

            duk_put_global_string(ctx, "ogle");
        }
    }
}
