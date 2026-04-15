#include "OpenGLRenderer.h"
#include "Camera.h"
#include "../managers/WorldManager.h"
#include "../world/WorldComponents.h"
#include "../Logger.h"
#include "../render/ProceduralTexture.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Windows.h>

OpenGLRenderer::OpenGLRenderer(int width, int height, OGLE::Camera& camera, WorldManager& worldManager)
    : m_shaderManager()
    , m_camera(camera)
    , m_worldManager(worldManager)
    , m_width(width)
    , m_height(height)
    , m_highlightedEntity(entt::null)
    , m_startTime(std::chrono::steady_clock::now())
{
}

void OpenGLRenderer::SetHighlightedEntity(OGLE::Entity entity)
{
    m_highlightedEntity = entity;
}

void OpenGLRenderer::SetSceneViewport(const glm::vec2& origin, const glm::vec2& size)
{
    // Метод оставлен для совместимости, но больше не влияет на рендеринг
    // Рендеринг теперь всегда происходит на весь экран
    UpdateSceneViewportState();
}

OpenGLRenderer::~OpenGLRenderer()
{
    DestroyShadowResources();

    if (m_gridVAO != 0) { glDeleteVertexArrays(1, &m_gridVAO); m_gridVAO = 0; }
    if (m_gridVBO != 0) { glDeleteBuffers(1, &m_gridVBO); m_gridVBO = 0; }
    if (m_gridIBO != 0) { glDeleteBuffers(1, &m_gridIBO); m_gridIBO = 0; }
    if (m_gizmoVAO != 0) { glDeleteVertexArrays(1, &m_gizmoVAO); m_gizmoVAO = 0; }
    if (m_gizmoVBO != 0) { glDeleteBuffers(1, &m_gizmoVBO); m_gizmoVBO = 0; }
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

    std::string vertexShaderSrc, fragmentShaderSrc, shadowVertexShaderSrc, shadowFragmentShaderSrc;
    
    try {
        vertexShaderSrc = m_shaderManager.LoadShaderSource("assets/shaders/default.vs");
        fragmentShaderSrc = m_shaderManager.LoadShaderSource("assets/shaders/default.fs");
        shadowVertexShaderSrc = m_shaderManager.LoadShaderSource("assets/shaders/shadow.vs");
        shadowFragmentShaderSrc = m_shaderManager.LoadShaderSource("assets/shaders/shadow.fs");
    }
    catch (const std::exception& e) {
        LOG_ERROR("OpenGLRenderer: Failed to load shader sources: " + std::string(e.what()));
        return false;
    }

    if (!m_shaderManager.loadVertexShader("default_vert", vertexShaderSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: loadVertexShader failed");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("default_frag", fragmentShaderSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: loadFragmentShader failed");
        return false;
    }
    if (!m_shaderManager.linkProgram("default", "default_vert", "default_frag")) {
        LOG_ERROR("OpenGLRenderer: linkProgram failed");
        return false;
    }
    if (!m_shaderManager.loadVertexShader("shadow_vs", shadowVertexShaderSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: shadow vertex shader failed");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("shadow_fs", shadowFragmentShaderSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: shadow fragment shader failed");
        return false;
    }
    if (!m_shaderManager.linkProgram("shadow_depth", "shadow_vs", "shadow_fs")) {
        LOG_ERROR("OpenGLRenderer: shadow program link failed");
        return false;
    }
    m_shaderManager.SetGlobalInstance(&m_shaderManager);

    // Инициализация compute shader'ов для процедурной генерации текстур
    if (!OGLE::ProceduralTexture::InitializeShaders()) {
        LOG_INFO("OpenGLRenderer: procedural texture shaders initialization completed");
    }

    if (!InitializeShadowResources()) {
        LOG_ERROR("OpenGLRenderer: failed to initialize shadow map resources");
        return false;
    }

    if (!InitializeGrid()) {
        LOG_ERROR("OpenGLRenderer: failed to initialize grid");
        return false;
    }

    if (!InitializeGizmo()) {
        LOG_ERROR("OpenGLRenderer: failed to initialize axis gizmo");
        return false;
    }

    m_gridInitialized = true;
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
    UpdateSceneViewportState();
    LOG_DEBUG("OpenGLRenderer::Resize set viewport " + std::to_string(m_width) + "x" + std::to_string(m_height));
}

void OpenGLRenderer::UpdateSceneViewportState()
{
    float aspectRatio = 1.0f;
    if (m_height > 0) {
        aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    }

    m_camera.SetAspectRatio(aspectRatio);
}

bool OpenGLRenderer::InitializeGrid()
{
    const float size = 500.0f;
    const float y = 0.0f;
    const float vertices[] = {
        -size, y, -size,
         size, y, -size,
         size, y,  size,
        -size, y,  size,
    };
    const unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);
    glGenBuffers(1, &m_gridIBO);

    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gridIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    const std::string gridVsSrc = m_shaderManager.LoadShaderSource("assets/shaders/grid.vs");
    const std::string gridFsSrc = m_shaderManager.LoadShaderSource("assets/shaders/grid.fs");
    if (gridVsSrc.empty() || gridFsSrc.empty()) {
        LOG_ERROR("OpenGLRenderer: failed to load grid shader sources");
        return false;
    }

    if (!m_shaderManager.loadVertexShader("grid_vs", gridVsSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: failed to compile grid vertex shader");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("grid_fs", gridFsSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: failed to compile grid fragment shader");
        return false;
    }
    if (!m_shaderManager.linkProgram("grid", "grid_vs", "grid_fs")) {
        LOG_ERROR("OpenGLRenderer: failed to link grid program");
        return false;
    }

    m_gridProgram = m_shaderManager.getProgram("grid");
    LOG_INFO("OpenGLRenderer: grid initialized");
    return true;
}

bool OpenGLRenderer::InitializeGizmo()
{
    const float length = 5.0f;
    const float vertices[] = {
        0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length,
    };

    glGenVertexArrays(1, &m_gizmoVAO);
    glGenBuffers(1, &m_gizmoVBO);

    glBindVertexArray(m_gizmoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glBindVertexArray(0);

    const std::string lineVsSrc = m_shaderManager.LoadShaderSource("assets/shaders/debug_line.vs");
    const std::string lineFsSrc = m_shaderManager.LoadShaderSource("assets/shaders/debug_line.fs");
    if (lineVsSrc.empty() || lineFsSrc.empty()) {
        LOG_ERROR("OpenGLRenderer: failed to load debug line shader sources");
        return false;
    }

    if (!m_shaderManager.loadVertexShader("debug_line_vs", lineVsSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: failed to compile debug line vertex shader");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("debug_line_fs", lineFsSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: failed to compile debug line fragment shader");
        return false;
    }
    if (!m_shaderManager.linkProgram("debug_line", "debug_line_vs", "debug_line_fs")) {
        LOG_ERROR("OpenGLRenderer: failed to link debug line program");
        return false;
    }

    m_gizmoProgram = m_shaderManager.getProgram("debug_line");
    LOG_INFO("OpenGLRenderer: gizmo initialized");
    return true;
}

void OpenGLRenderer::RenderGrid()
{
    if (!m_gridInitialized || m_gridProgram == 0) {
        return;
    }

    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();
    const glm::vec3 cameraPosition = m_camera.GetPosition();

    glUseProgram(m_gridProgram);

    const GLint vpLocation = glGetUniformLocation(m_gridProgram, "uViewProjection");
    if (vpLocation >= 0) {
        glUniformMatrix4fv(vpLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    const GLint cameraLocation = glGetUniformLocation(m_gridProgram, "uCameraPosition");
    if (cameraLocation >= 0) {
        glUniform3f(cameraLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    }

    const GLint fadeLocation = glGetUniformLocation(m_gridProgram, "uFadeDistance");
    if (fadeLocation >= 0) {
        glUniform1f(fadeLocation, 80.0f);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(m_gridVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void OpenGLRenderer::RenderGizmo()
{
    if (!m_gridInitialized || m_gizmoProgram == 0) {
        return;
    }

    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();
    glUseProgram(m_gizmoProgram);

    const GLint vpLocation = glGetUniformLocation(m_gizmoProgram, "uViewProjection");
    const GLint colorLocation = glGetUniformLocation(m_gizmoProgram, "uColor");
    if (vpLocation >= 0) {
        glUniformMatrix4fv(vpLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(m_gizmoVAO);

    if (colorLocation >= 0) { glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f); }
    glDrawArrays(GL_LINES, 0, 2);
    if (colorLocation >= 0) { glUniform4f(colorLocation, 0.3f, 1.0f, 0.3f, 1.0f); }
    glDrawArrays(GL_LINES, 2, 2);
    if (colorLocation >= 0) { glUniform4f(colorLocation, 0.3f, 0.3f, 1.0f, 1.0f); }
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
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

    // Упрощенный рендеринг на весь экран
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_showGrid) {
        RenderGrid();
    }

    if (!m_shaderManager.useProgram("default")) {
        return;
    }

    const GLint lightSpaceMatrixLocation = m_shaderManager.getUniformLocation("default", "uLightSpaceMatrix");
    const GLint viewPositionLocation = m_shaderManager.getUniformLocation("default", "uViewPosition");
    const GLint hasDirectionalLightLocation = m_shaderManager.getUniformLocation("default", "uHasDirectionalLight");
    const GLint directionalLightDirectionLocation = m_shaderManager.getUniformLocation("default", "uDirectionalLightDirection");
    const GLint directionalLightColorLocation = m_shaderManager.getUniformLocation("default", "uDirectionalLightColor");
    const GLint directionalLightIntensityLocation = m_shaderManager.getUniformLocation("default", "uDirectionalLightIntensity");
    const GLint directionalLightCastsShadowsLocation = m_shaderManager.getUniformLocation("default", "uDirectionalLightCastsShadows");
    const GLint pointLightCountLocation = m_shaderManager.getUniformLocation("default", "uPointLightCount");
    const GLint pointLightPositionsLocation = m_shaderManager.getUniformLocation("default", "uPointLightPositions[0]");
    const GLint pointLightColorsLocation = m_shaderManager.getUniformLocation("default", "uPointLightColors[0]");
    const GLint pointLightIntensitiesLocation = m_shaderManager.getUniformLocation("default", "uPointLightIntensities[0]");
    const GLint pointLightRangesLocation = m_shaderManager.getUniformLocation("default", "uPointLightRanges[0]");
    const GLint shadowMapLocation = m_shaderManager.getUniformLocation("default", "uShadowMap");
    const GLint selectionTintLocation = m_shaderManager.getUniformLocation("default", "uSelectionTint");
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

    auto SetProgramGlobalUniforms = [&](const std::string& programName) {
        const GLint hasDirectionalLightLocation = m_shaderManager.getUniformLocation(programName, "uHasDirectionalLight");
        const GLint directionalLightDirectionLocation = m_shaderManager.getUniformLocation(programName, "uDirectionalLightDirection");
        const GLint directionalLightColorLocation = m_shaderManager.getUniformLocation(programName, "uDirectionalLightColor");
        const GLint directionalLightIntensityLocation = m_shaderManager.getUniformLocation(programName, "uDirectionalLightIntensity");
        const GLint directionalLightCastsShadowsLocation = m_shaderManager.getUniformLocation(programName, "uDirectionalLightCastsShadows");
        const GLint pointLightCountLocation = m_shaderManager.getUniformLocation(programName, "uPointLightCount");
        const GLint pointLightPositionsLocation = m_shaderManager.getUniformLocation(programName, "uPointLightPositions[0]");
        const GLint pointLightColorsLocation = m_shaderManager.getUniformLocation(programName, "uPointLightColors[0]");
        const GLint pointLightIntensitiesLocation = m_shaderManager.getUniformLocation(programName, "uPointLightIntensities[0]");
        const GLint pointLightRangesLocation = m_shaderManager.getUniformLocation(programName, "uPointLightRanges[0]");
        const GLint shadowMapLocationLocal = m_shaderManager.getUniformLocation(programName, "uShadowMap");
        const GLint viewPositionLocationLocal = m_shaderManager.getUniformLocation(programName, "uViewPosition");

        if (shadowMapLocationLocal >= 0) {
            glUniform1i(shadowMapLocationLocal, 2);
        }
        if (viewPositionLocationLocal >= 0) {
            const glm::vec3 cameraPosition = m_camera.GetPosition();
            glUniform3f(viewPositionLocationLocal, cameraPosition.x, cameraPosition.y, cameraPosition.z);
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
    };

    auto lightView = m_worldManager.GetActiveWorld().GetRegistry().view<OGLE::WorldObjectComponent, OGLE::TransformComponent, OGLE::LightComponent>();
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

    // Default program gets initial lighting and scene uniforms.
    std::string currentProgramName = "default";
    SetProgramGlobalUniforms(currentProgramName);

    auto worldView = m_worldManager.GetActiveWorld().GetRegistry().view<OGLE::WorldObjectComponent, OGLE::ModelComponent>();
    for (auto entity : worldView) {
        auto& worldObjectComponent = worldView.get<OGLE::WorldObjectComponent>(entity);
        auto& modelComponent = worldView.get<OGLE::ModelComponent>(entity);
        if (!worldObjectComponent.enabled || !worldObjectComponent.visible || !modelComponent.model) {
            continue;
        }

        const OGLE::Material* materialForRender = nullptr;
        if (const OGLE::MaterialComponent* materialComponent = m_worldManager.GetActiveWorld().GetMaterial(entity)) {
            materialForRender = &materialComponent->material;
        } else {
            materialForRender = &modelComponent.model->GetMaterial();
        }

        std::string requestedProgramName = "default";
        if (const OGLE::ShaderComponent* shaderComp = m_worldManager.GetActiveWorld().GetShader(entity)) {
            requestedProgramName = shaderComp->programName.empty() ? "default" : shaderComp->programName;
        } else if (materialForRender && !materialForRender->GetShaderProgram().empty()) {
            requestedProgramName = materialForRender->GetShaderProgram();
        }

        if (!m_shaderManager.hasProgram(requestedProgramName)) {
            requestedProgramName = "default";
        }

        if (requestedProgramName != currentProgramName) {
            if (!m_shaderManager.useProgram(requestedProgramName)) {
                m_shaderManager.useProgram("default");
                requestedProgramName = "default";
            }
            currentProgramName = requestedProgramName;
            SetProgramGlobalUniforms(currentProgramName);
        }

        const GLint locationMVP = m_shaderManager.getUniformLocation(currentProgramName, "uMVP");
        const GLint locationModel = m_shaderManager.getUniformLocation(currentProgramName, "uModel");
        const GLint locationSelectionMix = m_shaderManager.getUniformLocation(currentProgramName, "uSelectionMix");

        if (locationMVP >= 0) {
            const glm::mat4 mvp = viewProjection * modelComponent.model->GetModelMatrix();
            glUniformMatrix4fv(locationMVP, 1, GL_FALSE, glm::value_ptr(mvp));
        }
        if (locationModel >= 0) {
            glUniformMatrix4fv(locationModel, 1, GL_FALSE, glm::value_ptr(modelComponent.model->GetModelMatrix()));
        }
        if (locationSelectionMix >= 0) {
            glUniform1f(locationSelectionMix, entity == m_highlightedEntity ? 0.45f : 0.0f);
        }

        if (materialForRender) {
            materialForRender->Bind();// materialForRender->Bind(programHandle);
        }

        modelComponent.model->Draw();
    }

    if (m_showGrid) {
        RenderGizmo();
    }

    // Viewport уже установлен в начале метода Render

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
    auto lightView = m_worldManager.GetActiveWorld().GetRegistry().view<OGLE::WorldObjectComponent, OGLE::TransformComponent, OGLE::LightComponent>();
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
    const GLint lightMvpLocation = m_shaderManager.getUniformLocation("shadow_depth", "uLightMVP");
    const GLint modelLocation = m_shaderManager.getUniformLocation("shadow_depth", "uModel");

    glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    auto worldView = m_worldManager.GetActiveWorld().GetRegistry().view<OGLE::WorldObjectComponent, OGLE::ModelComponent>();
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
