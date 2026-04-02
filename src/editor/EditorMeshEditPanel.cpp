#include "editor/EditorMeshEditPanel.h"
#include "world/World.h"
#include "models/ModelEntity.h"
#include "models/MeshBuffer.h"
#include <imgui.h> // for ImGui
#include <cstdio>  // for snprintf

EditorMeshEditPanel::EditorMeshEditPanel() = default;

void EditorMeshEditPanel::ResetState()
{
    m_selectedMeshIndex = -1;
    m_selectedVertexIndex = -1;
    m_lastProcessedEntity = entt::null;
}

void EditorMeshEditPanel::BuildUi(OGLE::World& world, OGLE::Entity selectedEntity)
{
    if (!m_show) {
        return;
    }

    ImGui::Begin("Mesh Editor", &m_show);

    if (selectedEntity != m_lastProcessedEntity) {
        ResetState();
        m_lastProcessedEntity = selectedEntity;
    }

    if (!world.IsValid(selectedEntity)) {
        ImGui::Text("No entity selected.");
        ImGui::End();
        return;
    }

    auto* modelComponent = world.GetRegistry().try_get<OGLE::ModelComponent>(selectedEntity);
    if (!modelComponent || !modelComponent->model) {
        ImGui::Text("Selected entity has no model component.");
        ImGui::End();
        return;
    }

    auto& model = modelComponent->model;

    if (model.use_count() > 1) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::TextWrapped("This model is a shared resource used by %ld entities. To edit its vertices, you must first make it unique for this entity.", model.use_count());
        ImGui::PopStyleColor();
        if (ImGui::Button("Make Unique and Editable")) {
            world.MakeModelUnique(selectedEntity);
        }
        ImGui::End();
        return;
    }

    // Поскольку ModelEntity содержит только один меш, нам не нужен выбор меша.
    // Данные вершин хранятся в ModelEntity как std::vector<float>.
    // Каждая вершина состоит из 8 float: 3 (позиция) + 3 (нормаль) + 2 (UV).
    auto& verticesData = model->GetVertices();
    const size_t floatsPerVertex = 8;
    const size_t vertexCount = verticesData.size() / floatsPerVertex;

    if (vertexCount == 0) {
        ImGui::Text("Model has no vertices.");
        ImGui::End();
        return;
    }

    ImGui::Separator();
    ImGui::Text("Vertices: %zu", vertexCount);

    ImGui::BeginChild("VertexList", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, 0), true);
    for (int i = 0; i < vertexCount; ++i) {
        char label[64];
        snprintf(label, sizeof(label), "Vertex %d", i); // Использование snprintf для безопасности
        if (ImGui::Selectable(label, m_selectedVertexIndex == i)) {
            m_selectedVertexIndex = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginGroup();
    if (m_selectedVertexIndex >= 0 && m_selectedVertexIndex < vertexCount) {
        ImGui::Text("Selected Vertex: %d", m_selectedVertexIndex);

        // Индекс начала данных позиции для выбранной вершины
        const size_t positionOffset = m_selectedVertexIndex * floatsPerVertex;
        glm::vec3 pos = glm::vec3(
            verticesData[positionOffset],
            verticesData[positionOffset + 1],
            verticesData[positionOffset + 2]
        );

        ImGui::PushItemWidth(-1);
        if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) {
            // Обновляем данные в векторе m_vertices ModelEntity
            verticesData[positionOffset] = pos.x;
            verticesData[positionOffset + 1] = pos.y;
            verticesData[positionOffset + 2] = pos.z;
            model->UpdateGpuData(); // Обновляем GPU буфер
        }
        ImGui::PopItemWidth();
    } else {
        ImGui::Text("Select a vertex to edit its position.");
    }
    ImGui::EndGroup();

    ImGui::End();
}