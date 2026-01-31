// src/render/LightContext.cpp
#include "LightContext.h"
#include "shader/ShaderProgram.h"

namespace ogle {

void LightContext::ApplyToShader(const LightContext* ctx, ShaderProgram* shader, bool useLighting) {
    if (!ctx || !shader || !useLighting)
        return;

    if (ctx->numDirectional > 0) {
        const auto& d = ctx->directional[0];
        shader->SetVec3("uLightDir", d.direction);
        shader->SetVec3("uLightColor", d.color * d.intensity);
    } else {
        shader->SetVec3("uLightDir", glm::vec3(0.5f, -1.0f, 0.5f));
        shader->SetVec3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // Точечные и прожекторы можно выставить здесь для шейдеров с массивами:
    // shader->SetInt("uNumPointLights", ctx->numPoint);
    // for (int i = 0; i < ctx->numPoint; ++i) { ... }
}

} // namespace ogle
