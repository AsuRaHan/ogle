#include "LayerStack.h"
#include <algorithm>

LayerStack::LayerStack()
{
}

LayerStack::~LayerStack()
{
    for (Layer* layer : m_layers)
    {
        layer->OnDetach();
        delete layer;
    }
}

void LayerStack::PushLayer(Layer* layer)
{
    m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
    m_layerInsertIndex++;
    layer->OnAttach();
}

void LayerStack::PushOverlay(Layer* layer)
{
    m_layers.emplace_back(layer);
    layer->OnAttach();
}

void LayerStack::PopLayer(Layer* layer)
{
    auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
    if (it != m_layers.begin() + m_layerInsertIndex)
    {
        layer->OnDetach();
        m_layers.erase(it);
        m_layerInsertIndex--;
    }
}

void LayerStack::PopOverlay(Layer* layer)
{
    auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), layer);
    if (it != m_layers.end())
    {
        layer->OnDetach();
        m_layers.erase(it);
    }
}
