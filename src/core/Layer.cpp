#include "Layer.h"

Layer::Layer(const std::string& name)
    : m_debugName(name)
{
}

void Layer::OnAttach()
{
}

void Layer::OnDetach()
{
}

void Layer::OnUpdate(float deltaTime)
{
}

void Layer::OnRender()
{
}

void Layer::OnImGuiRender()
{
}
