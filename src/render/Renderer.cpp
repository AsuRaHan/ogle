#include "Renderer.h"

Renderer::Renderer()
{
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LESS);

glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);

// Опционально: сглаживание граней (если хочешь красивее)
glEnable(GL_MULTISAMPLE);  // антиалиасинг, если драйвер поддерживает
}

Renderer::~Renderer() = default;

void Renderer::Render(const Scene& scene, const Camera& camera)
{
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);            // стандартный режим: рисуем, если ближе к камере

    // Создаём шейдер один раз (лучше вынести в поле Renderer позже)
    static Shader shader("data/shaders/simple_vertex.glsl", "data/shaders/simple_fragment.glsl");

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix(1280.0f / 720.0f);

    scene.Draw(shader, view, projection);
}