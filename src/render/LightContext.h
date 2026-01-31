// src/render/LightContext.h
#pragma once

#include <glm/glm.hpp>

namespace ogle {

class ShaderProgram;

// Константы лимитов источников света (должны совпадать с GLSL)
constexpr int kMaxDirectionalLights = 1;
constexpr int kMaxPointLights = 8;
constexpr int kMaxSpotLights = 4;

// Данные одного направленного света для шейдера (world space)
struct DirectionalLightData {
    glm::vec3 direction{0.5f, -1.0f, 0.5f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

// Данные одного точечного света для шейдера (world space)
struct PointLightData {
    glm::vec3 position{0.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

// Данные одного прожектора для шейдера (world space)
struct SpotLightData {
    glm::vec3 position{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float innerAngle = 0.218f;
    float outerAngle = 0.305f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

// Контекст освещения: собранные данные по всем источникам в сцене для рендера.
struct LightContext {
    int numDirectional = 0;
    int numPoint = 0;
    int numSpot = 0;
    DirectionalLightData directional[kMaxDirectionalLights];
    PointLightData point[kMaxPointLights];
    SpotLightData spot[kMaxSpotLights];

    // Применить освещение к шейдеру (выставляет uLightDir, uLightColor и т.д.).
    // Если ctx == nullptr или useLighting == false, не меняет uniform'ы.
    static void ApplyToShader(const LightContext* ctx, ShaderProgram* shader, bool useLighting);
};

} // namespace ogle
