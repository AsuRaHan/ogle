#include "OpenGLRenderer.h"
#include "Camera.h"
#include "../world/World.h"
#include "../world/WorldComponents.h"
#include "../Logger.h"
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Windows.h>
// chrono needed for steady_clock used in Render
// chrono needed for steady_clock used in Render
#include <chrono>

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

OpenGLRenderer::~OpenGLRenderer() {
    // Ничего не требуется, уникальный указатель сам освободит ресурсы
}

bool OpenGLRenderer::Initialize() {
    LOG_INFO("OpenGLRenderer: инициализация");

    glEnable(GL_DEPTH_TEST);
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
        out vec3 vWorldNormal;
        out vec2 vTexCoord;
        void main() {
            vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
            vTexCoord = aTexCoord;
            gl_Position = uMVP * vec4(aPosition, 1.0);
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        in vec3 vWorldNormal;
        in vec2 vTexCoord;
        uniform sampler2D uDiffuseTexture;
        uniform int uHasDiffuseTexture;
        uniform vec3 uBaseColor;
        uniform vec3 uLightDirection;
        uniform vec3 uSelectionTint;
        uniform float uSelectionMix;
        out vec4 FragColor;
        void main() {
            vec3 albedo = uBaseColor;
            if (uHasDiffuseTexture == 1) {
                albedo *= texture(uDiffuseTexture, vTexCoord).rgb;
            }

            vec3 normal = normalize(vWorldNormal);
            float diffuse = max(dot(normal, normalize(-uLightDirection)), 0.0);
            float ambient = 0.25;
            vec3 litColor = albedo * (ambient + diffuse * 0.75);
            litColor = mix(litColor, uSelectionTint, uSelectionMix);
            FragColor = vec4(litColor, 1.0);
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

    m_scene = std::make_unique<DomoScene>();
    if (!m_scene->Initialize()) {
        LOG_ERROR("OpenGLRenderer: не удалось создать ресурсы DomoScene");
        return false;
    }

    return true;
}

void OpenGLRenderer::Resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
    m_camera.SetAspectRatio(static_cast<float>(m_width) / m_height);
    LOG_DEBUG("OpenGLRenderer::Resize set viewport " + std::to_string(m_width) + "x" + std::to_string(m_height));
}

void OpenGLRenderer::Render() {
    // 1. Проверка ошибок до очистки
    if (glGetError() != GL_NO_ERROR) LOG_ERROR("Pre‑clear error");

    // 2. Установим viewport и очистим
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. Обновляем камеру

    // 4. Выбираем программу
    if (!m_shaderManager.useProgram("default")) return;
    const GLuint program = m_shaderManager.getProgram("default");
    const GLint mvpLocation = glGetUniformLocation(program, "uMVP");
    const GLint modelLocation = glGetUniformLocation(program, "uModel");
    const GLint lightDirectionLocation = glGetUniformLocation(program, "uLightDirection");
    const GLint selectionTintLocation = glGetUniformLocation(program, "uSelectionTint");
    const GLint selectionMixLocation = glGetUniformLocation(program, "uSelectionMix");
    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();

    if (lightDirectionLocation >= 0) {
        glUniform3f(lightDirectionLocation, -0.4f, -1.0f, -0.3f);
    }
    if (selectionTintLocation >= 0) {
        glUniform3f(selectionTintLocation, 1.0f, 0.85f, 0.2f);
    }

    // 5. Параметры сцены
    // float timeSec = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_startTime).count();
    // if (m_scene) {
    //     m_scene->setSize(glm::vec3(1.f));
    //     m_scene->setPosition(glm::vec3(0.f));
    //     m_scene->setRotation(timeSec, glm::vec3(0.f,1.f,0.f));
    //     m_scene->Render(timeSec);
    // }

    auto worldView = m_world.GetRegistry().view<OGLE::WorldObjectComponent, OGLE::ModelComponent>();
    for (auto entity : worldView) {
        auto& worldObjectComponent = worldView.get<OGLE::WorldObjectComponent>(entity);
        auto& modelComponent = worldView.get<OGLE::ModelComponent>(entity);
        if (!worldObjectComponent.enabled || !worldObjectComponent.visible) {
            continue;
        }
        if (!modelComponent.model) {
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

        modelComponent.model->BindMaterial(program);

        modelComponent.model->Draw();
    }

    // 6. Проверка ошибок после рендера
    if (glGetError() != GL_NO_ERROR) LOG_ERROR("Post‑draw error");
}
