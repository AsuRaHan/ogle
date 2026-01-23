#include "Scene.h"
#include "Camera.h"
#include "Shader.h"

Scene::Scene()
{
    m_root = std::make_unique<Entity>("Root");
}

Scene::~Scene() = default;

void Scene::Update(double deltaTime)
{
    if (m_root) m_root->Update(deltaTime);
}

void Scene::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const
{
    if (m_root)
    {
        m_root->Draw(shader, view, projection);
    }
}