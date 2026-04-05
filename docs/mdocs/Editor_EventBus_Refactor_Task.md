# TASK: Refactor Editor to Use Event Bus

## Overview
Your editor UI currently calls **managers directly** from button click handlers. For example:
- Click "Load World" → directly calls `worldManager.LoadActiveWorld()`
- Click "Create Cube" → directly calls `worldManager.CreatePrimitive()`
- Click "Delete" → directly calls `physicsManager.RemoveBody()` + `world.DestroyEntity()`

This creates tight coupling: Editor panels know about WorldManager, PhysicsManager, ConfigManager.

**Goal:** When user clicks a button, the Editor **dispatches an event** through EventBus. App.cpp subscribes to these events and calls the managers. Editor panels no longer need direct write access to managers.

**Key principle:** 
- **Writes (user actions)** → Events
- **Reads (displaying data in UI)** → Direct read access is OK (panels still need WorldManager& for reading entity data)

---

## Files to Modify

| # | File | Action | What |
|---|------|--------|------|
| 1 | `core/Events.h` | **MODIFY** | Add editor event types |
| 2 | `editor/EditorState.h` | **MODIFY** | Add `EventBus*` pointer |
| 3 | `editor/Editor.cpp` | **MODIFY** | Replace button actions with event dispatch |
| 4 | `editor/EditorCreationPanel.cpp` | **MODIFY** | Replace Create button with event dispatch |
| 5 | `editor/EditorHierarchyPanel.cpp` | **MODIFY** | Replace Add/Delete buttons with event dispatch |
| 6 | `editor/EditorInspectorPanel.cpp` | **MODIFY** | Replace action buttons with event dispatch |
| 7 | `App.cpp` | **MODIFY** | Subscribe to editor events, set EventBus* in EditorState |

**NO new files are created.** All changes are modifications to existing files.

---

## STEP 1: Add Editor Events to `core/Events.h`

Open `core/Events.h`. If it does not exist yet (previous task may not have been done), create it with all events including the ones from this step. If it already exists, **append** these new events at the end of the file, inside the `namespace ogle { }` block.

**Add these event types:**

```cpp
    // ── Editor Events ──────────────────────────────────────────────────────

    struct EditorLoadWorldEvent
    {
        std::string path;
    };

    struct EditorSaveWorldEvent
    {
        std::string path;
    };

    struct EditorReloadDefaultWorldEvent {};

    struct EditorClearWorldEvent {};

    struct EditorPlayEvent {};

    struct EditorPauseEvent {};

    struct EditorStepEvent {};

    struct EditorCreateEntityEvent
    {
        enum class Type
        {
            EmptyObject,
            Cube,
            Sphere,
            Plane,
            ModelFromFile,
            DirectionalLight,
            PointLight
        };

        Type type;
        std::string name;
        std::string modelPath;
        std::string texturePath;
    };

    struct EditorDeleteEntityEvent
    {
        entt::entity entity;
    };

    struct EditorSpawnModelFromDragDropEvent
    {
        std::string assetPath;
    };

    struct EditorTransformChangedEvent
    {
        entt::entity entity;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    struct EditorNameChangedEvent
    {
        entt::entity entity;
        std::string name;
    };
```

**IMPORTANT:** Make sure the file includes `<glm/glm.hpp>` at the top if not already there, because `EditorTransformChangedEvent` uses `glm::vec3`.

---

## STEP 2: Add EventBus pointer to EditorState

Open `editor/EditorState.h`.

**Add this include at the top:**
```cpp
#include "core/EventBus.h"
```

**Add this member inside the `EditorState` struct:**
```cpp
    EventBus* eventBus = nullptr;
```

Place it near the top of the struct, before the other members. This allows all editor panels to dispatch events through `state.eventBus`.

---

## STEP 3: Set EventBus pointer in `App.cpp`

In `App.cpp`, find the `MainApplicationLayer::OnImGuiRender()` method.

**Add this line at the very beginning of `OnImGuiRender()`** (before any other code in that method):

```cpp
        auto& eventBus = m_app.GetEventBus();
        editor.GetState().eventBus = &eventBus;
```

This ensures the EventBus pointer is set in EditorState before any panel draws.

**Also add include at top of App.cpp** (if not already there):
```cpp
#include "core/Events.h"
```

---

## STEP 4: Add Event Subscriptions in `App.cpp`

In `App::Run()`, add these event subscriptions. Place them **after** all the manager initializations and **before** the main loop. A good location is right after `m_timeManager.Reset();` and before `m_window->Show(nCmdShow);`.

Add the following subscriptions inside `App::Run()`:

```cpp
    // ── Editor event subscriptions ─────────────────────────────────────
    m_eventBus.Subscribe<ogle::EditorLoadWorldEvent>([this](const ogle::EditorLoadWorldEvent& e) {
        m_configManager.GetConfig().world.path = e.path;
        m_configManager.Save();
        m_worldManager.LoadActiveWorld(e.path);
        m_editor.GetState().selectedEntity = entt::null;
        m_editor.GetState().bufferedEntity = entt::null;
        m_editor.GetState().textureEditingEntity = entt::null;
    });

    m_eventBus.Subscribe<ogle::EditorSaveWorldEvent>([this](const ogle::EditorSaveWorldEvent& e) {
        m_configManager.GetConfig().world.path = e.path;
        m_configManager.Save();
        m_worldManager.SaveActiveWorld(e.path);
    });

    m_eventBus.Subscribe<ogle::EditorReloadDefaultWorldEvent>([this](const ogle::EditorReloadDefaultWorldEvent&) {
        m_worldManager.CreateDefaultWorld();
        m_editor.GetState().selectedEntity = entt::null;
        m_editor.GetState().bufferedEntity = entt::null;
        m_editor.GetState().textureEditingEntity = entt::null;
    });

    m_eventBus.Subscribe<ogle::EditorClearWorldEvent>([this](const ogle::EditorClearWorldEvent&) {
        m_worldManager.ClearWorld();
        m_editor.GetState().selectedEntity = entt::null;
        m_editor.GetState().bufferedEntity = entt::null;
        m_editor.GetState().textureEditingEntity = entt::null;
    });

    m_eventBus.Subscribe<ogle::EditorPlayEvent>([this](const ogle::EditorPlayEvent&) {
        m_editor.GetState().simulationState = Editor::SimulationState::Playing;
    });

    m_eventBus.Subscribe<ogle::EditorPauseEvent>([this](const ogle::EditorPauseEvent&) {
        m_editor.GetState().simulationState = Editor::SimulationState::Paused;
    });

    m_eventBus.Subscribe<ogle::EditorStepEvent>([this](const ogle::EditorStepEvent&) {
        m_editor.GetState().simulationState = Editor::SimulationState::Paused;
        m_editor.GetState().stepSimulationRequested = true;
    });

    m_eventBus.Subscribe<ogle::EditorCreateEntityEvent>([this](const ogle::EditorCreateEntityEvent& e) {
        entt::entity created = entt::null;

        switch (e.type) {
            case ogle::EditorCreateEntityEvent::Type::EmptyObject:
                created = m_worldManager.CreateWorldObject(e.name, OGLE::WorldObjectKind::Generic).GetEntity();
                break;
            case ogle::EditorCreateEntityEvent::Type::Cube:
                created = m_worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Cube,
                    glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), e.texturePath);
                break;
            case ogle::EditorCreateEntityEvent::Type::Sphere:
                created = m_worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Sphere,
                    glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), e.texturePath);
                break;
            case ogle::EditorCreateEntityEvent::Type::Plane:
                created = m_worldManager.CreatePrimitive(e.name, OGLE::PrimitiveType::Plane,
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f, 1.0f, 5.0f), e.texturePath);
                break;
            case ogle::EditorCreateEntityEvent::Type::DirectionalLight:
                created = m_worldManager.CreateDirectionalLight(e.name, glm::vec3(-50.0f, 45.0f, 0.0f));
                break;
            case ogle::EditorCreateEntityEvent::Type::PointLight:
                created = m_worldManager.CreatePointLight(e.name, glm::vec3(0.0f, 1.5f, 0.0f));
                break;
            case ogle::EditorCreateEntityEvent::Type::ModelFromFile:
                created = m_worldManager.CreateModelFromFile(e.modelPath, OGLE::ModelType::DYNAMIC, e.name);
                if (created != entt::null && !e.texturePath.empty()) {
                    m_worldManager.SetEntityDiffuseTexture(created, e.texturePath);
                }
                break;
        }

        if (created != entt::null) {
            m_editor.GetState().selectedEntity = created;
            m_editor.GetState().bufferedEntity = entt::null;
            m_editor.GetState().textureEditingEntity = entt::null;
        }
    });

    m_eventBus.Subscribe<ogle::EditorDeleteEntityEvent>([this](const ogle::EditorDeleteEntityEvent& e) {
        if (e.entity != entt::null && m_worldManager.IsEntityValid(e.entity)) {
            m_physicsManager.RemoveBody(e.entity);
            m_worldManager.GetActiveWorld().DestroyEntity(e.entity);
        }
        if (e.entity == m_editor.GetState().selectedEntity) {
            m_editor.GetState().selectedEntity = entt::null;
            m_editor.GetState().bufferedEntity = entt::null;
            m_editor.GetState().textureEditingEntity = entt::null;
        }
    });

    m_eventBus.Subscribe<ogle::EditorSpawnModelFromDragDropEvent>([this](const ogle::EditorSpawnModelFromDragDropEvent& e) {
        // Reuse camera + spawn logic from the original drag-drop handler
        const auto& camera = m_cameraManager.GetCamera();
        const glm::vec3 spawnPosition = camera.GetPosition() + camera.GetFront() * 5.0f;
        const OGLE::Entity entity = m_worldManager.CreateModelFromFile(
            e.assetPath, OGLE::ModelType::DYNAMIC, BuildEditorEntityNameFromAssetPath(e.assetPath));

        if (entity != entt::null) {
            auto& world = m_worldManager.GetActiveWorld();
            if (auto* transform = world.GetComponent<OGLE::TransformComponent>(entity)) {
                world.SetTransform(entity, spawnPosition, transform->rotation, transform->scale);
            }
            m_editor.GetState().selectedEntity = entity;
            m_editor.GetState().bufferedEntity = entt::null;
            m_editor.GetState().textureEditingEntity = entt::null;
        }
    });

    m_eventBus.Subscribe<ogle::EditorTransformChangedEvent>([this](const ogle::EditorTransformChangedEvent& e) {
        if (e.entity != entt::null && m_worldManager.IsEntityValid(e.entity)) {
            m_worldManager.GetWorldObject(e.entity).SetTransform(e.position, e.rotation, e.scale);
        }
    });

    m_eventBus.Subscribe<ogle::EditorNameChangedEvent>([this](const ogle::EditorNameChangedEvent& e) {
        if (e.entity != entt::null && m_worldManager.IsEntityValid(e.entity)) {
            m_worldManager.GetWorldObject(e.entity).SetName(e.name);
        }
    });
```

**IMPORTANT:** For the `EditorSpawnModelFromDragDropEvent` handler, it uses `BuildEditorEntityNameFromAssetPath()` which is defined in `EditorAssetHelpers.h`. You may need to add this include at the top of `App.cpp`:
```cpp
#include "editor/EditorAssetHelpers.h"
```

**IMPORTANT:** For the `glm::vec3` in `EditorCreateEntityEvent` handler, make sure `App.cpp` already has `#include <glm/vec3.hpp>` (it should already have it from existing code).

---

## STEP 5: Refactor `Editor.cpp` — Replace button clicks with events

### 5a. Add include at top
```cpp
#include "core/Events.h"
```

### 5b. Replace "Load World" menu item (line 132-139)

Find this block:
```cpp
            if (ImGui::MenuItem("Load World")) {
                configManager.GetConfig().world.path = m_worldPathBuffer.data();
                configManager.Save();
                worldManager.LoadActiveWorld(m_worldPathBuffer.data());
                m_selectedEntity = entt::null;
                m_bufferedEntity = entt::null;
                m_textureEditingEntity = entt::null;
            }
```

Replace with:
```cpp
            if (ImGui::MenuItem("Load World")) {
                m_state.eventBus->Dispatch(ogle::EditorLoadWorldEvent{ m_worldPathBuffer.data() });
            }
```

### 5c. Replace "Save World" menu item (line 140-144)

Find this block:
```cpp
            if (ImGui::MenuItem("Save World")) {
                configManager.GetConfig().world.path = m_worldPathBuffer.data();
                configManager.Save();
                worldManager.SaveActiveWorld(m_worldPathBuffer.data());
            }
```

Replace with:
```cpp
            if (ImGui::MenuItem("Save World")) {
                m_state.eventBus->Dispatch(ogle::EditorSaveWorldEvent{ m_worldPathBuffer.data() });
            }
```

### 5d. Replace "Reload Default" menu item (line 145-150)

Find this block:
```cpp
            if (ImGui::MenuItem("Reload Default")) {
                worldManager.CreateDefaultWorld();
                m_selectedEntity = entt::null;
                m_bufferedEntity = entt::null;
                m_textureEditingEntity = entt::null;
            }
```

Replace with:
```cpp
            if (ImGui::MenuItem("Reload Default")) {
                m_state.eventBus->Dispatch(ogle::EditorReloadDefaultWorldEvent{});
            }
```

### 5e. Replace "Clear World" menu item (line 151-156)

Find this block:
```cpp
            if (ImGui::MenuItem("Clear World")) {
                worldManager.ClearWorld();
                m_selectedEntity = entt::null;
                m_bufferedEntity = entt::null;
                m_textureEditingEntity = entt::null;
            }
```

Replace with:
```cpp
            if (ImGui::MenuItem("Clear World")) {
                m_state.eventBus->Dispatch(ogle::EditorClearWorldEvent{});
            }
```

### 5f. Replace Simulation menu items (lines 160-173)

Find this block:
```cpp
        if (ImGui::BeginMenu("Simulation")) {
            const bool isPlaying = m_simulationState == SimulationState::Playing;
            if (ImGui::MenuItem("Play", nullptr, isPlaying)) {
                m_simulationState = SimulationState::Playing;
            }
            if (ImGui::MenuItem("Pause", nullptr, !isPlaying)) {
                m_simulationState = SimulationState::Paused;
            }
            if (ImGui::MenuItem("Step")) {
                m_simulationState = SimulationState::Paused;
                m_stepSimulationRequested = true;
            }
            ImGui::EndMenu();
        }
```

Replace with:
```cpp
        if (ImGui::BeginMenu("Simulation")) {
            const bool isPlaying = m_simulationState == SimulationState::Playing;
            if (ImGui::MenuItem("Play", nullptr, isPlaying)) {
                m_state.eventBus->Dispatch(ogle::EditorPlayEvent{});
            }
            if (ImGui::MenuItem("Pause", nullptr, !isPlaying)) {
                m_state.eventBus->Dispatch(ogle::EditorPauseEvent{});
            }
            if (ImGui::MenuItem("Step")) {
                m_state.eventBus->Dispatch(ogle::EditorStepEvent{});
            }
            ImGui::EndMenu();
        }
```

### 5g. Replace World window buttons (lines 211-238)

Find this block:
```cpp
        if (ImGui::Button("Load World")) {
            configManager.GetConfig().world.path = m_worldPathBuffer.data();
            configManager.Save();
            worldManager.LoadActiveWorld(m_worldPathBuffer.data());
            m_selectedEntity = entt::null;
            m_bufferedEntity = entt::null;
            m_textureEditingEntity = entt::null;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save World")) {
            configManager.GetConfig().world.path = m_worldPathBuffer.data();
            configManager.Save();
            worldManager.SaveActiveWorld(m_worldPathBuffer.data());
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload Default")) {
            worldManager.CreateDefaultWorld();
            m_selectedEntity = entt::null;
            m_bufferedEntity = entt::null;
            m_textureEditingEntity = entt::null;
        }

        if (ImGui::Button("Clear World")) {
            worldManager.ClearWorld();
            m_selectedEntity = entt::null;
            m_bufferedEntity = entt::null;
            m_textureEditingEntity = entt::null;
        }
```

Replace with:
```cpp
        if (ImGui::Button("Load World")) {
            m_state.eventBus->Dispatch(ogle::EditorLoadWorldEvent{ m_worldPathBuffer.data() });
        }
        ImGui::SameLine();
        if (ImGui::Button("Save World")) {
            m_state.eventBus->Dispatch(ogle::EditorSaveWorldEvent{ m_worldPathBuffer.data() });
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload Default")) {
            m_state.eventBus->Dispatch(ogle::EditorReloadDefaultWorldEvent{});
        }

        if (ImGui::Button("Clear World")) {
            m_state.eventBus->Dispatch(ogle::EditorClearWorldEvent{});
        }
```

### 5h. Replace Simulation buttons in World window (lines 240-249)

Find this block:
```cpp
        if (ImGui::Button(m_simulationState == SimulationState::Playing ? "Pause Simulation" : "Play Simulation")) {
            m_simulationState = (m_simulationState == SimulationState::Playing)
                ? SimulationState::Paused
                : SimulationState::Playing;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step Simulation")) {
            m_simulationState = SimulationState::Paused;
            m_stepSimulationRequested = true;
        }
```

Replace with:
```cpp
        if (ImGui::Button(m_simulationState == SimulationState::Playing ? "Pause Simulation" : "Play Simulation")) {
            m_state.eventBus->Dispatch(m_simulationState == SimulationState::Playing
                ? ogle::EditorPauseEvent{}
                : ogle::EditorPlayEvent{});
        }
        ImGui::SameLine();
        if (ImGui::Button("Step Simulation")) {
            m_state.eventBus->Dispatch(ogle::EditorStepEvent{});
        }
```

### 5i. Replace DragDrop handler (lines 254-277)

Find this block:
```cpp
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetContentBrowserAssetPayload())) {
                const char* assetPath = static_cast<const char*>(payload->Data);
                if (assetPath && IsEditorModelAssetPath(assetPath)) {
                    const std::string entityName = BuildEditorEntityNameFromAssetPath(assetPath);
                    const OGLE::Entity entity = worldManager.CreateModelFromFile(
                        assetPath,
                        OGLE::ModelType::DYNAMIC,
                        entityName);

                    if (entity != entt::null) {
                        const glm::vec3 spawnPosition = camera.GetPosition() + camera.GetFront() * 5.0f;
                        auto& world = worldManager.GetActiveWorld();
                        if (auto* transform = world.GetComponent<OGLE::TransformComponent>(entity)) {
                            world.SetTransform(entity, spawnPosition, transform->rotation, transform->scale);
                        }
                        m_selectedEntity = entity;
                        m_bufferedEntity = entt::null;
                        m_textureEditingEntity = entt::null;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
```

Replace with:
```cpp
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GetContentBrowserAssetPayload())) {
                const char* assetPath = static_cast<const char*>(payload->Data);
                if (assetPath && IsEditorModelAssetPath(assetPath)) {
                    m_state.eventBus->Dispatch(ogle::EditorSpawnModelFromDragDropEvent{ std::string(assetPath) });
                }
            }
            ImGui::EndDragDropTarget();
        }
```

### 5j. Simplify BuildUi signature

After all changes above, the `BuildUi` method no longer needs `PhysicsManager&` or `ConfigManager&` for writing. However, it still reads from them for display. So keep the signature **unchanged for now** — the reads are fine.

**DO NOT change the BuildUi signature.** The reads (displaying physics body count, config path, etc.) are acceptable direct accesses.

---

## STEP 6: Refactor `EditorCreationPanel.cpp`

### 6a. Add includes at top
```cpp
#include "core/Events.h"
#include "core/EventBus.h"
```

### 6b. Replace the Create button handler (lines 25-72)

Find this block:
```cpp
    if (ImGui::Button("Create")) {
        const std::string name = state.createNameBuffer[0] != '\0' ? state.createNameBuffer.data() : "NewObject";

        if (state.createKind == 0) {
            state.selectedEntity = worldManager.CreateWorldObject(name, OGLE::WorldObjectKind::Generic).GetEntity();
        } else if (state.createKind == 1) {
            state.selectedEntity = worldManager.CreatePrimitive(
                name,
                OGLE::PrimitiveType::Cube,
                glm::vec3(0.0f, 0.5f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                state.createTexturePathBuffer.data());
        } else if (state.createKind == 2) {
            state.selectedEntity = worldManager.CreatePrimitive(
                name,
                OGLE::PrimitiveType::Sphere,
                glm::vec3(0.0f, 0.5f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                state.createTexturePathBuffer.data());
        } else if (state.createKind == 3) {
            state.selectedEntity = worldManager.CreatePrimitive(
                name,
                OGLE::PrimitiveType::Plane,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(5.0f, 1.0f, 5.0f),
                state.createTexturePathBuffer.data());
        } else if (state.createKind == 5) {
            state.selectedEntity = worldManager.CreateDirectionalLight(
                name,
                glm::vec3(-50.0f, 45.0f, 0.0f));
        } else if (state.createKind == 6) {
            state.selectedEntity = worldManager.CreatePointLight(
                name,
                glm::vec3(0.0f, 1.5f, 0.0f));
        } else {
            state.selectedEntity = worldManager.CreateModelFromFile(
                state.createModelPathBuffer.data(),
                OGLE::ModelType::DYNAMIC,
                name);

            if (state.selectedEntity != entt::null && state.createTexturePathBuffer[0] != '\0') {
                worldManager.SetEntityDiffuseTexture(state.selectedEntity, state.createTexturePathBuffer.data());
            }
        }

        state.bufferedEntity = entt::null;
        state.textureEditingEntity = entt::null;
    }
```

Replace with:
```cpp
    if (ImGui::Button("Create")) {
        if (!state.eventBus) return;  // Safety check

        const std::string name = state.createNameBuffer[0] != '\0' ? state.createNameBuffer.data() : "NewObject";
        const std::string modelPath = state.createModelPathBuffer.data();
        const std::string texturePath = state.createTexturePathBuffer.data();

        ogle::EditorCreateEntityEvent event;
        event.name = name;
        event.modelPath = modelPath;
        event.texturePath = texturePath;

        switch (state.createKind) {
            case 0: event.type = ogle::EditorCreateEntityEvent::Type::EmptyObject; break;
            case 1: event.type = ogle::EditorCreateEntityEvent::Type::Cube; break;
            case 2: event.type = ogle::EditorCreateEntityEvent::Type::Sphere; break;
            case 3: event.type = ogle::EditorCreateEntityEvent::Type::Plane; break;
            case 5: event.type = ogle::EditorCreateEntityEvent::Type::DirectionalLight; break;
            case 6: event.type = ogle::EditorCreateEntityEvent::Type::PointLight; break;
            case 4: event.type = ogle::EditorCreateEntityEvent::Type::ModelFromFile; break;
            default: return;
        }

        state.eventBus->Dispatch(event);
    }
```

---

## STEP 7: Refactor `EditorHierarchyPanel.cpp`

### 7a. Add includes at top
```cpp
#include "core/Events.h"
#include "core/EventBus.h"
```

### 7b. Replace "Add Empty" button (line 37-40)

Find this block:
```cpp
    if (ImGui::Button("Add Empty")) {
        state.selectedEntity = worldManager.CreateWorldObject("EmptyObject", OGLE::WorldObjectKind::Generic).GetEntity();
        state.bufferedEntity = entt::null;
    }
```

Replace with:
```cpp
    if (ImGui::Button("Add Empty")) {
        if (state.eventBus) {
            state.eventBus->Dispatch(ogle::EditorCreateEntityEvent{
                ogle::EditorCreateEntityEvent::Type::EmptyObject,
                "EmptyObject",
                "",
                ""
            });
        }
    }
```

### 7c. Replace "Delete" button (lines 51-57)

Find this block:
```cpp
    if (ImGui::Button("Delete")) {
        physicsManager.RemoveBody(state.selectedEntity);
        worldManager.GetActiveWorld().DestroyEntity(state.selectedEntity);
        state.selectedEntity = entt::null;
        state.bufferedEntity = entt::null;
        state.textureEditingEntity = entt::null;
    }
```

Replace with:
```cpp
    if (ImGui::Button("Delete")) {
        if (state.eventBus) {
            state.eventBus->Dispatch(ogle::EditorDeleteEntityEvent{ state.selectedEntity });
        }
    }
```

### 7d. Replace context menu "Delete" (lines 89-100)

Find this block:
```cpp
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        physicsManager.RemoveBody(entity);
                        if (entity == state.selectedEntity) {
                            state.selectedEntity = entt::null;
                            state.bufferedEntity = entt::null;
                            state.textureEditingEntity = entt::null;
                        }
                        worldManager.GetActiveWorld().DestroyEntity(entity);
                        ImGui::EndPopup();
                        break;
                    }
                    ImGui::EndPopup();
                }
```

Replace with:
```cpp
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        if (state.eventBus) {
                            state.eventBus->Dispatch(ogle::EditorDeleteEntityEvent{ entity });
                        }
                        ImGui::EndPopup();
                        break;
                    }
                    ImGui::EndPopup();
                }
```

---

## STEP 8: Refactor `EditorInspectorPanel.cpp`

### 8a. Add includes at top
```cpp
#include "core/Events.h"
#include "core/EventBus.h"
```

### 8b. Replace "Apply Name" button (lines 54-56)

Find this block:
```cpp
    if (ImGui::Button("Apply Name")) {
        selectedObject.SetName(state.selectedNameBuffer.data());
    }
```

Replace with:
```cpp
    if (ImGui::Button("Apply Name")) {
        if (state.eventBus) {
            state.eventBus->Dispatch(ogle::EditorNameChangedEvent{
                state.selectedEntity,
                state.selectedNameBuffer.data()
            });
        }
    }
```

### 8c. Replace transform DragFloat handler (lines 76-82)

Find this block:
```cpp
    if (transformChanged) {
        selectedObject.SetTransform(position, rotation, scale);
    }
```

Replace with:
```cpp
    if (transformChanged) {
        if (state.eventBus) {
            state.eventBus->Dispatch(ogle::EditorTransformChangedEvent{
                state.selectedEntity,
                position,
                rotation,
                scale
            });
        }
    }
```

### 8d. Replace ImGuizmo handler (lines 111-116)

Find this block:
```cpp
    if (ImGuizmo::IsUsing())
    {
        glm::vec3 newPosition, newScale, newRotation;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), glm::value_ptr(newPosition), glm::value_ptr(newRotation), glm::value_ptr(newScale));
        selectedObject.SetTransform(newPosition, newRotation, newScale);
    }
```

Replace with:
```cpp
    if (ImGuizmo::IsUsing())
    {
        glm::vec3 newPosition, newScale, newRotation;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), glm::value_ptr(newPosition), glm::value_ptr(newRotation), glm::value_ptr(newScale));
        if (state.eventBus) {
            state.eventBus->Dispatch(ogle::EditorTransformChangedEvent{
                state.selectedEntity,
                newPosition,
                newRotation,
                newScale
            });
        }
    }
```

### 8e. Replace "Delete Selected" button at the bottom (lines 473-479)

Find this block:
```cpp
    if (ImGui::Button("Delete Selected")) {
        physicsManager.RemoveBody(state.selectedEntity);
        world.DestroyEntity(state.selectedEntity);
        state.selectedEntity = entt::null;
        state.bufferedEntity = entt::null;
        state.textureEditingEntity = entt::null;
    }
```

Replace with:
```cpp
    if (ImGui::Button("Delete Selected")) {
        if (state.eventBus) {
            state.eventBus->Dispatch(ogle::EditorDeleteEntityEvent{ state.selectedEntity });
        }
    }
```

### 8f. Leave the rest UNCHANGED

**DO NOT change** these parts of the Inspector — they modify component data directly through ECS, which is a low-level operation better kept as direct access for now:
- Apply Material button
- Apply Light button
- Apply Physics button (the physics body creation part: `physicsManager.AddBoxBody`, etc.)
- Remove Physics button
- Apply Skeleton button
- Remove Skeleton button
- Apply Animation button
- Remove Animation button
- Apply Script button
- Remove Script button
- Add Material / Add Physics / Add Light / Add Skeleton / Add Animation / Add Script buttons
- Save/Load Materials buttons
- Save/Load Animations buttons
- Material drag-drop handler

These can be converted to events in a **future task**. For now, focus on the main editor actions listed above.

---

## Summary of Changes

### Events added (11 new types):
| Event | Triggered by |
|-------|-------------|
| `EditorLoadWorldEvent` | File → Load World, World panel → Load button |
| `EditorSaveWorldEvent` | File → Save World, World panel → Save button |
| `EditorReloadDefaultWorldEvent` | File → Reload Default, World panel → Reload button |
| `EditorClearWorldEvent` | File → Clear World, World panel → Clear button |
| `EditorPlayEvent` | Simulation → Play, World panel → Play button |
| `EditorPauseEvent` | Simulation → Pause, World panel → Pause button |
| `EditorStepEvent` | Simulation → Step, World panel → Step button |
| `EditorCreateEntityEvent` | Creation panel → Create, Hierarchy → Add Empty |
| `EditorDeleteEntityEvent` | Hierarchy → Delete, Inspector → Delete Selected |
| `EditorSpawnModelFromDragDropEvent` | World panel → drag-drop model |
| `EditorTransformChangedEvent` | Inspector → DragFloat, Inspector → ImGuizmo |
| `EditorNameChangedEvent` | Inspector → Apply Name |

### What stays as direct calls (for now):
- Reading entity data for display (hierarchy list, inspector properties)
- Component-level apply buttons (Material, Light, Physics, etc.)
- Component add/remove buttons
- Material/Animation library save/load

---

## Build and Test

```
PS E:\my_proj\ogle> .\build.bat Release
```

**Expected result:**
- Compiles without errors.
- All editor functionality works exactly as before.
- Clicking "Load World" loads the world (via event now).
- Clicking "Create Cube" creates a cube (via event now).
- Clicking "Delete" deletes the entity (via event now).
- Play/Pause/Step still work (via events now).
- Transform dragging and ImGuizmo still work (via events now).

**If you get compile errors:**
1. Check that `core/Events.h` has all the event types defined inside `namespace ogle { }`
2. Check that `core/Events.h` includes `<glm/glm.hpp>` and `<string>`
3. Check that `editor/EditorState.h` includes `"core/EventBus.h"` and has `EventBus* eventBus = nullptr;`
4. Check that `App.cpp` includes `"core/Events.h"` and `"editor/EditorAssetHelpers.h"`
5. Make sure all event dispatches use `ogle::` prefix (e.g. `ogle::EditorLoadWorldEvent`)
6. If `EditorState` does not use a namespace, check how it's defined and adjust accordingly

---

## Why This Matters

Before this change, the **Editor class and all its panels** directly called:
- `WorldManager` (Load/Save/Create/Delete/Clear)
- `PhysicsManager` (RemoveBody/AddBody)
- `ConfigManager` (Save config)

After this change, Editor panels only know about:
- `EventBus*` (to dispatch events)
- `WorldManager&` (to READ data for display only)

The **write operations** (modifying world state) now flow through events, which means:
- You can add undo/redo by intercepting events
- You can add logging/telemetry by subscribing to events
- You can add validation before world modifications
- New systems (e.g. audio, analytics) can subscribe without modifying editor code
