#pragma once

#include "world/WorldComponents.h"

namespace OGLE
{
    class World;
}

class EditorMeshEditPanel
{
public:
    EditorMeshEditPanel();
    void BuildUi(OGLE::World& world, OGLE::Entity selectedEntity);

    bool m_show = false;

private:
    int m_selectedMeshIndex = -1;
    int m_selectedVertexIndex = -1;
    OGLE::Entity m_lastProcessedEntity = entt::null;

    void ResetState();
};