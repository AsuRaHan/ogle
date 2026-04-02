#include "OpenGLRenderer.h"
#include "Camera.h"
#include "../world/World.h"
#include "../world/WorldComponents.h"
#include "../Logger.h"

#include <array>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Windows.h>

OpenGLRenderer::OpenGLRenderer(int width, int height, ogle::Camera& camera, OGLE::World& world)
    : m_shaderManager()
    , m_camera(camera)
    , m_world(world)
    , m_width(width)
    , m_height(height)
    , m_highlightedEntity(entt::null)
    , m_scene(nullptr)
    , m_startTime(std::chrono::steady_clock::now())
{
}

void OpenGLRenderer::SetHighlightedEntity(OGLE::Entity entity)
{
    m_highlightedEntity = entity;
}

OpenGLRenderer::~OpenGLRenderer()
{
    DestroyShadowResources();
}

bool OpenGLRenderer::Initialize()
{
    LOG_INFO("OpenGLRenderer: initialization");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.05f, 0.12f, 0.17f, 1.0f);

    Resize(m_width, m_height);

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;
        uniform mat4 uMVP;
        uniform mat4 uModel;
        uniform mat4 uLightSpaceMatrix;
        uniform vec2 uUvTiling;
        uniform vec2 uUvOffset;
        out vec3 vWorldNormal;
        out vec3 vWorldPosition;
        out vec4 vLightSpacePosition;
        out vec2 vTexCoord;
        void main() {
            vec4 worldPosition = uModel * vec4(aPosition, 1.0);
            vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
            vWorldPosition = worldPosition.xyz;
            vLightSpacePosition = uLightSpaceMatrix * worldPosition;
            vTexCoord = aTexCoord * uUvTiling + uUvOffset;
            gl_Position = uMVP * vec4(aPosition, 1.0);
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        in vec3 vWorldNormal;
        in vec3 vWorldPosition;
        in vec4 vLightSpacePosition;
        in vec2 vTexCoord;
        uniform sampler2D uDiffuseTexture;
        uniform sampler2D uEmissiveTexture;
        uniform sampler2D uShadowMap;
        uniform int uHasDiffuseTexture;
        uniform int uHasEmissiveTexture;
        uniform int uHasDirectionalLight;
        uniform int uDirectionalLightCastsShadows;
        uniform int uPointLightCount;
        uniform vec3 uBaseColor;
        uniform vec3 uEmissiveColor;
        uniform vec3 uViewPosition;
        uniform vec3 uDirectionalLightDirection;
        uniform vec3 uDirectionalLightColor;
        uniform float uDirectionalLightIntensity;
        uniform vec3 uPointLightPositions[4];
        uniform vec3 uPointLightColors[4];
        uniform float uPointLightIntensities[4];
        uniform float uPointLightRanges[4];
        uniform float uRoughness;
        uniform float uMetallic;
        uniform float uAlphaCutoff;
        uniform vec3 uSelectionTint;
        uniform float uSelectionMix;
        out vec4 FragColor;

        float ComputeShadowFactor(vec4 lightSpacePosition, vec3 normal, vec3 lightDirection) {
            vec3 projected = lightSpacePosition.xyz / max(lightSpacePosition.w, 0.0001);
            projected = projected * 0.5 + 0.5;
            if (projected.z > 1.0 || projected.x < 0.0 || projected.x > 1.0 || projected.y < 0.0 || projected.y > 1.0) {
                return 0.0;
            }

            float closestDepth = texture(uShadowMap, projected.xy).r;
            float currentDepth = projected.z;
            float bias = max(0.0008, 0.0035 * (1.0 - max(dot(normal, lightDirection), 0.0)));
            return currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }

        void main() {
            // One primary shadowed directional light plus a few point lights keeps the renderer understandable.
            vec4 diffuseSample = vec4(1.0);
            if (uHasDiffuseTexture == 1) {
                diffuseSample = texture(uDiffuseTexture, vTexCoord);
            }
            if (diffuseSample.a < uAlphaCutoff) {
                discard;
            }

            vec3 albedo = uBaseColor * diffuseSample.rgb;
            vec3 emissive = uEmissiveColor;
            if (uHasEmissiveTexture == 1) {
                emissive *= texture(uEmissiveTexture, vTexCoord).rgb;
            }

            vec3 normal = normalize(vWorldNormal);
            vec3 viewDirection = normalize(uViewPosition - vWorldPosition);
            float ambient = 0.25;
            float shininess = mix(4.0, 64.0, 1.0 - uRoughness);
            float specularStrength = mix(0.04, 1.0, uMetallic);
            vec3 litColor = albedo * ambient;

            if (uHasDirectionalLight == 1) {
                vec3 lightDirection = normalize(-uDirectionalLightDirection);
                vec3 halfVector = normalize(lightDirection + viewDirection);
                float diffuse = max(dot(normal, lightDirection), 0.0);
                float specular = pow(max(dot(normal, halfVector), 0.0), shininess) * specularStrength;
                float shadow = uDirectionalLightCastsShadows == 1
                    ? ComputeShadowFactor(vLightSpacePosition, normal, lightDirection)
                    : 0.0;
                litColor +=
                    (1.0 - shadow) *
                    (albedo * diffuse + vec3(specular)) *
                    uDirectionalLightColor *
                    uDirectionalLightIntensity;
            }

            for (int i = 0; i < uPointLightCount; ++i) {
                vec3 toLight = uPointLightPositions[i] - vWorldPosition;
                float distanceToLight = length(toLight);
                if (distanceToLight > uPointLightRanges[i]) {
                    continue;
                }

                vec3 lightDirection = normalize(toLight);
                vec3 halfVector = normalize(lightDirection + viewDirection);
                float diffuse = max(dot(normal, lightDirection), 0.0);
                float specular = pow(max(dot(normal, halfVector), 0.0), shininess) * specularStrength;
                float attenuation = 1.0 - clamp(distanceToLight / max(uPointLightRanges[i], 0.0001), 0.0, 1.0);
                attenuation *= attenuation;
                litColor +=
                    (albedo * diffuse + vec3(specular)) *
                    uPointLightColors[i] *
                    uPointLightIntensities[i] *
                    attenuation;
            }

            litColor += emissive;
            litColor = mix(litColor, uSelectionTint, uSelectionMix);
            FragColor = vec4(litColor, diffuseSample.a);
        }
    )";

    const char* shadowVertexShaderSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPosition;
        uniform mat4 uLightMVP;
        uniform mat4 uModel;
        void main() {
            gl_Position = uLightMVP * uModel * vec4(aPosition, 1.0);
        }
    )";

    const char* shadowFragmentShaderSrc = R"(
        #version 330 core
        void main() {
        }
    )";

    if (!m_shaderManager.loadVertexShader("default_vs", vertexShaderSrc)) {
        LOG_ERROR("OpenGLRenderer: loadVertexShader failed");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("default_fs", fragmentShaderSrc)) {
        LOG_ERROR("OpenGLRenderer: loadFragmentShader failed");
        return false;
    }
    if (!m_shaderManager.linkProgram("default", "default_vs", "default_fs")) {
        LOG_ERROR("OpenGLRenderer: linkProgram failed");
        return false;
    }
    if (!m_shaderManager.loadVertexShader("shadow_vs", shadowVertexShaderSrc)) {
        LOG_ERROR("OpenGLRenderer: shadow vertex shader failed");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("shadow_fs", shadowFragmentShaderSrc)) {
        LOG_ERROR("OpenGLRenderer: shadow fragment shader failed");
        return false;
    }
    if (!m_shaderManager.linkProgram("shadow_depth", "shadow_vs", "shadow_fs")) {
        LOG_ERROR("OpenGLRenderer: shadow program link failed");
        return false;
    }
    if (!InitializeShadowResources()) {
        LOG_ERROR("OpenGLRenderer: failed to initialize shadow map resources");
        return false;
    }

    m_scene = std::make_unique<DomoScene>();
    if (!m_scene->Initialize()) {
        LOG_ERROR("OpenGLRenderer: failed to create DomoScene resources");
        return false;
    }

    return true;
}

void OpenGLRenderer::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
    m_camera.SetAspectRatio(static_cast<float>(m_width) / m_height);
    LOG_DEBUG("OpenGLRenderer::Resize set viewport " + std::to_string(m_width) + "x" + std::to_string(m_height));
}

void OpenGLRenderer::Render()
{
    if (glGetError() != GL_NO_ERROR) {
        LOG_ERROR("Pre-clear error");
    }

    LightingState lightingState;
    CollectLightingState(lightingState);
    if (lightingState.hasDirectionalLight && lightingState.castsShadows) {
        RenderShadowPass(lightingState);
    }

    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_shaderManager.useProgram("default")) {
        return;
    }

    const GLuint program = m_shaderManager.getProgram("default");
    const GLint mvpLocation = glGetUniformLocation(program, "uMVP");
    const GLint modelLocation = glGetUniformLocation(program, "uModel");
    const GLint lightSpaceMatrixLocation = glGetUniformLocation(program, "uLightSpaceMatrix");
    const GLint viewPositionLocation = glGetUniformLocation(program, "uViewPosition");
    const GLint hasDirectionalLightLocation = glGetUniformLocation(program, "uHasDirectionalLight");
    const GLint directionalLightDirectionLocation = glGetUniformLocation(program, "uDirectionalLightDirection");
    const GLint directionalLightColorLocation = glGetUniformLocation(program, "uDirectionalLightColor");
    const GLint directionalLightIntensityLocation = glGetUniformLocation(program, "uDirectionalLightIntensity");
    const GLint directionalLightCastsShadowsLocation = glGetUniformLocation(program, "uDirectionalLightCastsShadows");
    const GLint pointLightCountLocation = glGetUniformLocation(program, "uPointLightCount");
    const GLint pointLightPositionsLocation = glGetUniformLocation(program, "uPointLightPositions[0]");
    const GLint pointLightColorsLocation = glGetUniformLocation(program, "uPointLightColors[0]");
    const GLint pointLightIntensitiesLocation = glGetUniformLocation(program, "uPointLightIntensities[0]");
    const GLint pointLightRangesLocation = glGetUniformLocation(program, "uPointLightRanges[0]");
    const GLint shadowMapLocation = glGetUniformLocation(program, "uShadowMap");
    const GLint selectionTintLocation = glGetUniformLocation(program, "uSelectionTint");
    const GLint selectionMixLocation = glGetUniformLocation(program, "uSelectionMix");
    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();

    if (lightSpaceMatrixLocation >= 0) {
        glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightingState.lightSpaceMatrix));
    }
    if (viewPositionLocation >= 0) {
        const glm::vec3 cameraPosition = m_camera.GetPosition();
        glUniform3f(viewPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    }
    if (hasDirectionalLightLocation >= 0) {
        glUniform1i(hasDirectionalLightLocation, lightingState.hasDirectionalLight ? 1 : 0);
    }
    if (directionalLightDirectionLocation >= 0) {
        glUniform3f(
            directionalLightDirectionLocation,
            lightingState.directionalDirection.x,
            lightingState.directionalDirection.y,
            lightingState.directionalDirection.z);
    }
    if (directionalLightColorLocation >= 0) {
        glUniform3f(
            directionalLightColorLocation,
            lightingState.directionalColor.x,
            lightingState.directionalColor.y,
            lightingState.directionalColor.z);
    }
    if (directionalLightIntensityLocation >= 0) {
        glUniform1f(directionalLightIntensityLocation, lightingState.directionalIntensity);
    }
    if (directionalLightCastsShadowsLocation >= 0) {
        glUniform1i(directionalLightCastsShadowsLocation, lightingState.castsShadows ? 1 : 0);
    }
    if (shadowMapLocation >= 0) {
        glUniform1i(shadowMapLocation, 2);
    }

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);
    glActiveTexture(GL_TEXTURE0);

    std::array<glm::vec3, 4> pointLightPositions{};
    std::array<glm::vec3, 4> pointLightColors{};
    std::array<float, 4> pointLightIntensities{};
    std::array<float, 4> pointLightRanges{};
    int pointLightCount = 0;

    auto lightView = m_world.GetRegistry().view<OGLE::WorldObjectComponent, OGLE::TransformComponent, OGLE::LightComponent>();
    for (auto entity : lightView) {
        const auto& worldObject = lightView.get<OGLE::WorldObjectComponent>(entity);
        const auto& transform = lightView.get<OGLE::TransformComponent>(entity);
        const auto& light = lightView.get<OGLE::LightComponent>(entity);
        if (!worldObject.enabled || light.type != OGLE::LightType::Point) {
            continue;
        }
        if (pointLightCount >= static_cast<int>(pointLightPositions.size())) {
            break;
        }

        pointLightPositions[pointLightCount] = transform.position;
        pointLightColors[pointLightCount] = light.color;
        pointLightIntensities[pointLightCount] = light.intensity;
        pointLightRanges[pointLightCount] = light.range;
        ++pointLightCount;
    }

    if (pointLightCountLocation >= 0) {
        glUniform1i(pointLightCountLocation, pointLightCount);
    }
    if (pointLightPositionsLocation >= 0 && pointLightCount > 0) {
        glUniform3fv(pointLightPositionsLocation, pointLightCount, glm::value_ptr(pointLightPositions[0]));
    }
    if (pointLightColorsLocation >= 0 && pointLightCount > 0) {
        glUniform3fv(pointLightColorsLocation, pointLightCount, glm::value_ptr(pointLightColors[0]));
    }
    if (pointLightIntensitiesLocation >= 0 && pointLightCount > 0) {
        glUniform1fv(pointLightIntensitiesLocation, pointLightCount, pointLightIntensities.data());
    }
    if (pointLightRangesLocation >= 0 && pointLightCount > 0) {
        glUniform1fv(pointLightRangesLocation, pointLightCount, pointLightRanges.data());
    }
    if (selectionTintLocation >= 0) {
        glUniform3f(selectionTintLocation, 1.0f, 0.85f, 0.2f);
    }

    auto worldView = m_world.GetRegistry().view<OGLE::WorldObjectComponent, OGLE::ModelComponent>();
    for (auto entity : worldView) {
        auto& worldObjectComponent = worldView.get<OGLE::WorldObjectComponent>(entity);
        auto& modelComponent = worldView.get<OGLE::ModelComponent>(entity);
        if (!worldObjectComponent.enabled || !worldObjectComponent.visible || !modelComponent.model) {
            continue;
        }

        if (mvpLocation >= 0) {
            const glm::mat4 mvp = viewProjection * modelComponent.model->GetModelMatrix();
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        }
        if (modelLocation >= 0) {
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelComponent.model->GetModelMatrix()));
        }
        if (selectionMixLocation >= 0) {
            glUniform1f(selectionMixLocation, entity == m_highlightedEntity ? 0.45f : 0.0f);
        }

        if (const OGLE::MaterialComponent* materialComponent = m_world.GetMaterial(entity)) {
            materialComponent->material.Bind(program);
        } else {
            modelComponent.model->BindMaterial(program);
        }
        modelComponent.model->Draw();
    }

    if (glGetError() != GL_NO_ERROR) {
        LOG_ERROR("Post-draw error");
    }
}

bool OpenGLRenderer::InitializeShadowResources()
{
    DestroyShadowResources();

    glGenFramebuffers(1, &m_shadowFramebuffer);
    glGenTextures(1, &m_shadowDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_shadowMapSize, m_shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowDepthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return complete;
}

void OpenGLRenderer::DestroyShadowResources()
{
    if (m_shadowDepthTexture != 0) {
        glDeleteTextures(1, &m_shadowDepthTexture);
        m_shadowDepthTexture = 0;
    }
    if (m_shadowFramebuffer != 0) {
        glDeleteFramebuffers(1, &m_shadowFramebuffer);
        m_shadowFramebuffer = 0;
    }
}

glm::vec3 OpenGLRenderer::RotationToDirection(const glm::vec3& rotationDegrees) const
{
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
}

void OpenGLRenderer::CollectLightingState(LightingState& lightingState)
{
    auto lightView = m_world.GetRegistry().view<OGLE::WorldObjectComponent, OGLE::TransformComponent, OGLE::LightComponent>();
    for (auto entity : lightView) {
        const auto& worldObject = lightView.get<OGLE::WorldObjectComponent>(entity);
        const auto& transform = lightView.get<OGLE::TransformComponent>(entity);
        const auto& light = lightView.get<OGLE::LightComponent>(entity);
        if (!worldObject.enabled || light.type != OGLE::LightType::Directional) {
            continue;
        }

        if (!lightingState.hasDirectionalLight || light.primary) {
            lightingState.hasDirectionalLight = true;
            lightingState.directionalDirection = RotationToDirection(transform.rotation);
            lightingState.directionalColor = light.color;
            lightingState.directionalIntensity = light.intensity;
            lightingState.castsShadows = light.castShadows;
            if (light.primary) {
                break;
            }
        }
    }

    if (!lightingState.hasDirectionalLight) {
        lightingState.directionalDirection = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f));
        lightingState.directionalColor = glm::vec3(1.0f);
        lightingState.directionalIntensity = 1.25f;
        lightingState.castsShadows = false;
    }

    // Stable scene-wide shadow projection keeps shadows readable without cascade complexity.
    const glm::vec3 lightDirection = glm::normalize(lightingState.directionalDirection);
    const glm::vec3 lightPosition = -lightDirection * 20.0f;
    const glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 lightProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 60.0f);
    lightingState.lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
}

void OpenGLRenderer::RenderShadowPass(const LightingState& lightingState)
{
    if (m_shadowFramebuffer == 0 || !m_shaderManager.useProgram("shadow_depth")) {
        return;
    }

    const GLuint program = m_shaderManager.getProgram("shadow_depth");
    const GLint lightMvpLocation = glGetUniformLocation(program, "uLightMVP");
    const GLint modelLocation = glGetUniformLocation(program, "uModel");

    glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    auto worldView = m_world.GetRegistry().view<OGLE::WorldObjectComponent, OGLE::ModelComponent>();
    for (auto entity : worldView) {
        const auto& worldObjectComponent = worldView.get<OGLE::WorldObjectComponent>(entity);
        const auto& modelComponent = worldView.get<OGLE::ModelComponent>(entity);
        if (!worldObjectComponent.enabled || !worldObjectComponent.visible || !modelComponent.model) {
            continue;
        }

        if (lightMvpLocation >= 0) {
            glUniformMatrix4fv(lightMvpLocation, 1, GL_FALSE, glm::value_ptr(lightingState.lightSpaceMatrix));
        }
        if (modelLocation >= 0) {
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelComponent.model->GetModelMatrix()));
        }

        modelComponent.model->Draw();
    }

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);
}
