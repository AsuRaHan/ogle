#pragma once

#include <DuktapeCpp.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <entt/entt.hpp>

namespace OGLE {
    using Entity = entt::entity;
}

// Teach duktape-cpp how to handle glm::vec3
namespace duktape {
    template<>
    struct type_info<glm::vec3> {
        static bool is(duk_context *ctx, int index) {
            return duk_is_object(ctx, index);
        }

        static glm::vec3 get(duk_context *ctx, int index) {
            glm::vec3 result;
            duk_get_prop_string(ctx, index, "x");
            result.x = (float)duk_to_number(ctx, -1);
            duk_get_prop_string(ctx, index, "y");
            result.y = (float)duk_to_number(ctx, -1);
            duk_get_prop_string(ctx, index, "z");
            result.z = (float)duk_to_number(ctx, -1);
            duk_pop_3(ctx);
            return result;
        }

        static void push(duk_context *ctx, const glm::vec3 &value) {
            duk_idx_t obj_idx = duk_push_object(ctx);
            duk_push_number(ctx, value.x);
            duk_put_prop_string(ctx, obj_idx, "x");
            duk_push_number(ctx, value.y);
            duk_put_prop_string(ctx, obj_idx, "y");
            duk_push_number(ctx, value.z);
            duk_put_prop_string(ctx, obj_idx, "z");
        }
    };

    // Teach duktape-cpp how to handle glm::vec2
    template<>
    struct type_info<glm::vec2> {
        static bool is(duk_context *ctx, int index) {
            return duk_is_object(ctx, index);
        }

        static glm::vec2 get(duk_context *ctx, int index) {
            glm::vec2 result;
            duk_get_prop_string(ctx, index, "x");
            result.x = (float)duk_to_number(ctx, -1);
            duk_get_prop_string(ctx, index, "y");
            result.y = (float)duk_to_number(ctx, -1);
            duk_pop_2(ctx);
            return result;
        }

        static void push(duk_context *ctx, const glm::vec2 &value) {
            duk_idx_t obj_idx = duk_push_object(ctx);
            duk_push_number(ctx, value.x);
            duk_put_prop_string(ctx, obj_idx, "x");
            duk_push_number(ctx, value.y);
            duk_put_prop_string(ctx, obj_idx, "y");
        }
    };

    // Teach duktape-cpp how to handle OGLE::Entity (entt::entity)
    template<>
    struct type_info<OGLE::Entity> {
        static OGLE::Entity get(duk_context *ctx, int index) {
            return (OGLE::Entity)duk_require_uint(ctx, index);
        }

        static void push(duk_context *ctx, const OGLE::Entity &value) {
            duk_push_uint(ctx, (duk_uint_t)entt::to_integral(value));
        }
    };
}