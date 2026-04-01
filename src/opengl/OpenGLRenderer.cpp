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
    , m_scene(nullptr)
    , m_startTime(std::chrono::steady_clock::now())
{
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
        layout(location = 1) in vec3 aColor;
        uniform mat4 uMVP;
        out vec3 vColor;
        void main() {
            vColor = aColor;
            gl_Position = uMVP * vec4(aPosition, 1.0);
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
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
    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();

    // 5. Параметры сцены
    // float timeSec = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_startTime).count();
    // if (m_scene) {
    //     m_scene->setSize(glm::vec3(1.f));
    //     m_scene->setPosition(glm::vec3(0.f));
    //     m_scene->setRotation(timeSec, glm::vec3(0.f,1.f,0.f));
    //     m_scene->Render(timeSec);
    // }

    auto worldView = m_world.GetRegistry().view<OGLE::ModelComponent>();
    for (auto entity : worldView) {
        auto& modelComponent = worldView.get<OGLE::ModelComponent>(entity);
        if (!modelComponent.model) {
            continue;
        }

        if (mvpLocation >= 0) {
            const glm::mat4 mvp = viewProjection * modelComponent.model->GetModelMatrix();
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        }

        modelComponent.model->Draw();
    }

    // 6. Проверка ошибок после рендера
    if (glGetError() != GL_NO_ERROR) LOG_ERROR("Post‑draw error");
}
