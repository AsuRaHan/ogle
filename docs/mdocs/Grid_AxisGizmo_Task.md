# TASK: Implement 3D Grid and Origin Axis Gizmo

## Overview
Add two visual elements to the 3D editor viewport:

1. **Infinite ground grid** — a grid on the XZ plane (Y=0) with:
   - Minor lines every 1 unit (dark gray)
   - Major lines every 10 units (brighter)
   - Red tint on the X axis line
   - Blue tint on the Z axis line
   - Smooth distance-based fade (disappears far from camera)
   - Procedural shader — no line geometry needed, renders on a single flat quad

2. **Origin axis gizmo** — three colored lines from the world origin:
   - Red line along +X (5 units)
   - Green line along +Y (5 units)
   - Blue line along +Z (5 units)
   - "X", "Y", "Z" text labels rendered as ImGui overlay
   - Always visible (no depth test)

Both features are togglable from the editor World window via a "Show Grid" checkbox.

**This task creates 4 NEW shader files and modifies 3 EXISTING files.**

---

## Files Overview

| # | File | Action | What |
|---|------|--------|------|
| 1 | `assets/shaders/grid.vs` | **CREATE** | Grid vertex shader |
| 2 | `assets/shaders/grid.fs` | **CREATE** | Grid fragment shader (procedural) |
| 3 | `assets/shaders/debug_line.vs` | **CREATE** | Debug line vertex shader (reusable) |
| 4 | `assets/shaders/debug_line.fs` | **CREATE** | Debug line fragment shader (solid color) |
| 5 | `OpenGLRenderer.h` | **MODIFY** | Add grid/gizmo members |
| 6 | `OpenGLRenderer.cpp` | **MODIFY** | Initialize and render grid + gizmo |
| 7 | `editor/Editor.cpp` | **MODIFY** | Add grid toggle + axis labels overlay |

**NO C++ manager changes. NO App.h/App.cpp changes. NO new classes.**

---

## STEP 1: Create shader files

All shader files go in the `assets/shaders/` directory (same folder as `default.vs`, `shadow.vs`, etc.).

### FILE 1: `assets/shaders/grid.vs`

```glsl
#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uViewProjection;
out vec3 vWorldPosition;
void main() {
    vWorldPosition = aPosition;
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
```

### FILE 2: `assets/shaders/grid.fs`

```glsl
#version 330 core
in vec3 vWorldPosition;
uniform vec3 uCameraPosition;
uniform float uFadeDistance;
out vec4 FragColor;

void main() {
    vec2 coord = vWorldPosition.xz;

    // Anti-aliased line function using screen-space derivatives
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float minorLine = 1.0 - min(min(grid.x, grid.y), 1.0);

    // Major grid every 10 units
    vec2 majorCoord = coord / 10.0;
    vec2 majorDerivative = fwidth(majorCoord);
    vec2 majorGrid = abs(fract(majorCoord - 0.5) - 0.5) / majorDerivative;
    float majorLine = 1.0 - min(min(majorGrid.x, majorGrid.y), 1.0);

    // Axis highlight lines
    float axisXLine = 1.0 - min(abs(vWorldPosition.z) / derivative.y, 1.0);
    float axisZLine = 1.0 - min(abs(vWorldPosition.x) / derivative.x, 1.0);

    // Distance-based fade
    float dist = length(vWorldPosition.xz - uCameraPosition.xz);
    float fade = 1.0 - smoothstep(uFadeDistance * 0.6, uFadeDistance, dist);

    // Compose color and alpha
    vec3 color = vec3(0.25, 0.25, 0.28);
    float alpha = minorLine * 0.25;

    // Major grid (brighter)
    color = mix(color, vec3(0.4, 0.4, 0.45), majorLine * 0.8);
    alpha = max(alpha, majorLine * 0.35);

    // X axis (red)
    color = mix(color, vec3(0.7, 0.15, 0.15), axisXLine);
    alpha = max(alpha, axisXLine * 0.7);

    // Z axis (blue)
    color = mix(color, vec3(0.15, 0.15, 0.7), axisZLine);
    alpha = max(alpha, axisZLine * 0.7);

    alpha *= fade;

    if (alpha < 0.005) discard;

    FragColor = vec4(color, alpha);
}
```

### FILE 3: `assets/shaders/debug_line.vs`

```glsl
#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uViewProjection;
void main() {
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
```

### FILE 4: `assets/shaders/debug_line.fs`

```glsl
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    FragColor = uColor;
}
```

---

## STEP 2: Modify `OpenGLRenderer.h`

### 2a. Add members

Find the private section. Add these members after the existing `m_startTime` member:

```cpp
    // ── Grid ────────────────────────────────────────────────────────────
    GLuint m_gridVAO = 0;
    GLuint m_gridVBO = 0;
    GLuint m_gridIBO = 0;
    GLuint m_gridProgram = 0;

    // ── Axis Gizmo ──────────────────────────────────────────────────────
    GLuint m_gizmoVAO = 0;
    GLuint m_gizmoVBO = 0;
    GLuint m_gizmoProgram = 0;

    // ── Debug ───────────────────────────────────────────────────────────
    bool m_showGrid = true;
    bool m_gridInitialized = false;
```

### 2b. Add private methods

Add these method declarations in the private section (after `RotationToDirection`):

```cpp
    bool InitializeGrid();
    bool InitializeGizmo();
    void RenderGrid();
    void RenderGizmo();
```

### 2c. Add public methods

Add these in the public section (after `SetHighlightedEntity`):

```cpp
    void SetShowGrid(bool show) { m_showGrid = show; }
    bool IsGridVisible() const { return m_showGrid; }
```

---

## STEP 3: Modify `OpenGLRenderer.cpp`

### 3a. Add the grid/gizmo initialization in `Initialize()`

Find the end of the `Initialize()` method. It currently ends with:

```cpp
    if (!InitializeShadowResources()) {
        LOG_ERROR("OpenGLRenderer: failed to initialize shadow map resources");
        return false;
    }

    return true;
}
```

**Replace the last `return true;` with this block** (insert between the shadow check and `return true;`):

```cpp
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
```

### 3b. Add the four new methods

Add these methods **before** the `Render()` method. Place them after `Resize()` and before `Render()`:

```cpp
bool OpenGLRenderer::InitializeGrid()
{
    // Create a large flat quad on the XZ plane at Y=0
    // The procedural shader handles all the visual detail
    float size = 500.0f;
    float y = 0.0f;
    float vertices[] = {
        -size, y, -size,
         size, y, -size,
         size, y,  size,
        -size, y,  size,
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);
    glGenBuffers(1, &m_gridIBO);

    glBindVertexArray(m_gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gridIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Load and compile the grid shader program
    std::string gridVsSrc = m_shaderManager.LoadShaderSource("assets/shaders/grid.vs");
    std::string gridFsSrc = m_shaderManager.LoadShaderSource("assets/shaders/grid.fs");

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
        LOG_ERROR("OpenGLRenderer: failed to link grid shader program");
        return false;
    }
    m_gridProgram = m_shaderManager.getProgram("grid");

    LOG_INFO("OpenGLRenderer: grid initialized");
    return true;
}

bool OpenGLRenderer::InitializeGizmo()
{
    // Three axis lines from origin, 5 units each
    // X = Red, Y = Green, Z = Blue
    float length = 5.0f;
    float vertices[] = {
        // X axis (Red)
        0.0f, 0.0f, 0.0f,
        length, 0.0f, 0.0f,
        // Y axis (Green)
        0.0f, 0.0f, 0.0f,
        0.0f, length, 0.0f,
        // Z axis (Blue)
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, length,
    };

    glGenVertexArrays(1, &m_gizmoVAO);
    glGenBuffers(1, &m_gizmoVBO);

    glBindVertexArray(m_gizmoVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    // Load and compile the debug line shader program
    std::string lineVsSrc = m_shaderManager.LoadShaderSource("assets/shaders/debug_line.vs");
    std::string lineFsSrc = m_shaderManager.LoadShaderSource("assets/shaders/debug_line.fs");

    if (lineVsSrc.empty() || lineFsSrc.empty()) {
        LOG_ERROR("OpenGLRenderer: failed to load debug_line shader sources");
        return false;
    }

    if (!m_shaderManager.loadVertexShader("debug_line_vs", lineVsSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: failed to compile debug_line vertex shader");
        return false;
    }
    if (!m_shaderManager.loadFragmentShader("debug_line_fs", lineFsSrc.c_str())) {
        LOG_ERROR("OpenGLRenderer: failed to compile debug_line fragment shader");
        return false;
    }
    if (!m_shaderManager.linkProgram("debug_line", "debug_line_vs", "debug_line_fs")) {
        LOG_ERROR("OpenGLRenderer: failed to link debug_line shader program");
        return false;
    }
    m_gizmoProgram = m_shaderManager.getProgram("debug_line");

    LOG_INFO("OpenGLRenderer: axis gizmo initialized");
    return true;
}
```

### 3c. Add the RenderGrid and RenderGizmo methods

Add these methods **before** the `Render()` method (after the Initialize methods):

```cpp
void OpenGLRenderer::RenderGrid()
{
    if (!m_gridInitialized || m_gridProgram == 0)
        return;

    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();
    const glm::vec3 cameraPosition = m_camera.GetPosition();

    glUseProgram(m_gridProgram);

    const GLint vpLocation = glGetUniformLocation(m_gridProgram, "uViewProjection");
    if (vpLocation >= 0) {
        glUniformMatrix4fv(vpLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    const GLint camPosLocation = glGetUniformLocation(m_gridProgram, "uCameraPosition");
    if (camPosLocation >= 0) {
        glUniform3f(camPosLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    }

    const GLint fadeLocation = glGetUniformLocation(m_gridProgram, "uFadeDistance");
    if (fadeLocation >= 0) {
        glUniform1f(fadeLocation, 80.0f);
    }

    // Grid is transparent: depth test ON (hidden behind objects),
    // depth write OFF (doesn't block objects behind it), blend ON
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(m_gridVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void OpenGLRenderer::RenderGizmo()
{
    if (!m_gridInitialized || m_gizmoProgram == 0)
        return;

    const glm::mat4 viewProjection = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix();

    glUseProgram(m_gizmoProgram);

    const GLint vpLocation = glGetUniformLocation(m_gizmoProgram, "uViewProjection");
    if (vpLocation >= 0) {
        glUniformMatrix4fv(vpLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    const GLint colorLocation = glGetUniformLocation(m_gizmoProgram, "uColor");

    // Axis gizmo always on top: disable depth test
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Optional: thicker lines. Note: glLineWidth > 1.0 is deprecated in core OpenGL
    // and may not work on all drivers. If it doesn't work, lines will be 1px thin.
    glLineWidth(2.0f);

    glBindVertexArray(m_gizmoVBO);

    // X axis — Red
    if (colorLocation >= 0) {
        glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f);
    }
    glDrawArrays(GL_LINES, 0, 2);

    // Y axis — Green
    if (colorLocation >= 0) {
        glUniform4f(colorLocation, 0.3f, 1.0f, 0.3f, 1.0f);
    }
    glDrawArrays(GL_LINES, 2, 2);

    // Z axis — Blue
    if (colorLocation >= 0) {
        glUniform4f(colorLocation, 0.3f, 0.3f, 1.0f, 1.0f);
    }
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
```

### 3d. Call RenderGrid and RenderGizmo from Render()

In the `Render()` method, find this section (the very beginning of the render pass):

```cpp
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_shaderManager.useProgram("default")) {
        return;
    }
```

**Insert the grid and gizmo rendering between `glClear` and `useProgram("default")`:**

```cpp
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render grid BEFORE scene objects (grid has depth write OFF)
    if (m_showGrid) {
        RenderGrid();
    }

    if (!m_shaderManager.useProgram("default")) {
        return;
    }
```

Now find the end of the `Render()` method. After the main render loop (after the `worldView` for loop that draws all objects), find:

```cpp
    if (glGetError() != GL_NO_ERROR) {
        LOG_ERROR("Post-draw error");
    }
}
```

**Insert the gizmo rendering BEFORE this error check:**

```cpp
    // Render axis gizmo AFTER scene objects (always on top)
    if (m_showGrid) {
        RenderGizmo();
    }

    if (glGetError() != GL_NO_ERROR) {
        LOG_ERROR("Post-draw error");
    }
```

### 3e. Cleanup in destructor

In the `~OpenGLRenderer()` destructor, add cleanup after `DestroyShadowResources()`:

Find:
```cpp
OpenGLRenderer::~OpenGLRenderer()
{
    DestroyShadowResources();
}
```

Replace with:
```cpp
OpenGLRenderer::~OpenGLRenderer()
{
    DestroyShadowResources();

    if (m_gridVAO != 0) { glDeleteVertexArrays(1, &m_gridVAO); m_gridVAO = 0; }
    if (m_gridVBO != 0) { glDeleteBuffers(1, &m_gridVBO); m_gridVBO = 0; }
    if (m_gridIBO != 0) { glDeleteBuffers(1, &m_gridIBO); m_gridIBO = 0; }

    if (m_gizmoVAO != 0) { glDeleteVertexArrays(1, &m_gizmoVAO); m_gizmoVAO = 0; }
    if (m_gizmoVBO != 0) { glDeleteBuffers(1, &m_gizmoVBO); m_gizmoVBO = 0; }
}
```

---

## STEP 4: Modify `Editor.cpp` — Add grid toggle and axis labels

### 4a. Add "Show Grid" checkbox in the World window

Find this section in `BuildUi()` (inside the World window `ImGui::Begin("World", ...)` block):

```cpp
        ImGui::Separator();
        m_creationPanel.Draw(m_state, worldManager);
```

**Insert the checkbox BEFORE the separator:**

```cpp
        ImGui::Separator();

        // Grid toggle — communicates with OpenGLRenderer through RenderManager
        if (ImGui::Checkbox("Show Grid", &m_state.showGrid)) {
            // The actual toggle is applied in OnImGuiRender via MainApplicationLayer
            // For now, we store the state and apply it there
        }

        ImGui::Separator();
        m_creationPanel.Draw(m_state, worldManager);
```

### 4b. Add axis labels as ImGui overlay

Find the `OnImGuiRender()` method in `MainApplicationLayer` (in `App.cpp` — see notes below).

**IMPORTANT:** The axis labels need to be drawn during the ImGui render phase. They need the camera's view-projection matrix to project 3D positions to screen coordinates.

In `App.cpp`, find the `MainApplicationLayer::OnImGuiRender()` method. Add the axis label drawing code at the **end** of this method, **before the closing brace**:

Add this code block at the end of `OnImGuiRender()`:

```cpp
        // ── Axis Gizmo Labels ───────────────────────────────────────────
        {
            const ogle::Camera& camera = cameraManager.GetCamera();
            const glm::mat4 vp = camera.GetProjectionMatrix() * camera.GetViewMatrix();

            auto projectToScreen = [&](const glm::vec3& worldPos) -> ImVec2 {
                glm::vec4 clip = vp * glm::vec4(worldPos, 1.0f);
                if (clip.w <= 0.001f) return ImVec2(-9999.0f, -9999.0f);
                glm::vec2 ndc(clip.x / clip.w, -clip.y / clip.w);
                const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                return ImVec2(
                    (ndc.x * 0.5f + 0.5f) * displaySize.x,
                    (ndc.y * 0.5f + 0.5f) * displaySize.y
                );
            };

            ImDrawList* drawList = ImGui::GetForegroundDrawList();

            // Label positions: slightly past the end of each axis line (5.5 units)
            ImVec2 xScreen = projectToScreen(glm::vec3(5.8f, 0.0f, 0.0f));
            ImVec2 yScreen = projectToScreen(glm::vec3(0.0f, 5.8f, 0.0f));
            ImVec2 zScreen = projectToScreen(glm::vec3(0.0f, 0.0f, 5.8f));

            // Only draw labels that are in front of the camera
            ImFont* font = ImGui::GetFont();
            float fontSize = font->GetFontSize();

            if (xScreen.x > -999.0f) {
                drawList->AddText(ImVec2(xScreen.x - fontSize * 0.3f, xScreen.y - fontSize * 0.5f),
                    IM_COL32(255, 80, 80, 255), "X");
            }
            if (yScreen.x > -999.0f) {
                drawList->AddText(ImVec2(yScreen.x - fontSize * 0.3f, yScreen.y - fontSize * 0.5f),
                    IM_COL32(80, 255, 80, 255), "Y");
            }
            if (zScreen.x > -999.0f) {
                drawList->AddText(ImVec2(zScreen.x - fontSize * 0.3f, zScreen.y - fontSize * 0.5f),
                    IM_COL32(80, 80, 255, 255), "Z");
            }
        }
```

**IMPORTANT:** This code needs `#include "opengl/Camera.h"` and `#include <imgui.h>` in `App.cpp`. The file already includes both (Camera.h is used elsewhere, imgui.h is included through ImGuiManager).

### 4c. Wire the grid toggle to OpenGLRenderer

In `App.cpp`, find the `MainApplicationLayer::OnImGuiRender()` method. At the very beginning of this method (before any other code), add:

```cpp
        auto& renderManager = m_app.GetRenderManager();
        renderManager.SetShowGrid(m_app.GetEditor().GetState().showGrid);
```

### 4d. Add `showGrid` to EditorState

Open `editor/EditorState.h` and add this member inside the struct:

```cpp
    bool showGrid = true;
```

Place it near the top of the struct, near `enabled` and the other boolean flags.

**Also update the Editor.h alias list.** In `Editor.h`, find the alias section and add:

```cpp
    bool& m_showGrid = m_state.showGrid;
```

Wait — actually, the checkbox in step 4a already writes directly to `m_state.showGrid`. And in step 4c, we read from `m_app.GetEditor().GetState().showGrid`. So the Editor.h alias is optional but consistent with the existing pattern. Add it if you want to be consistent:

Find the alias block in `Editor.h`:
```cpp
    bool& m_showWorldWindow = m_state.showWorldWindow;
```

Add after it:
```cpp
    bool& m_showGrid = m_state.showGrid;
```

But actually, looking at the checkbox code in step 4a, I used `m_state.showGrid` directly (which is the same as `m_showGrid` through the alias). Either way works. The alias just provides the `m_showGrid` name which is consistent with other aliases.

---

## Summary of ALL Changes

| File | Change |
|------|--------|
| `assets/shaders/grid.vs` | **NEW** — 10 lines |
| `assets/shaders/grid.fs` | **NEW** — 50 lines (procedural grid) |
| `assets/shaders/debug_line.vs` | **NEW** — 8 lines |
| `assets/shaders/debug_line.fs` | **NEW** — 7 lines (solid color) |
| `OpenGLRenderer.h` | Add ~10 members and 5 method declarations |
| `OpenGLRenderer.cpp` | Add ~200 lines (init + render + cleanup) |
| `Editor.cpp` | Add "Show Grid" checkbox (~5 lines) |
| `App.cpp` (MainApplicationLayer) | Add grid toggle wire + axis labels (~30 lines) |
| `EditorState.h` | Add `bool showGrid = true;` |
| `Editor.h` | Add alias `bool& m_showGrid = m_state.showGrid;` |

**Total: ~330 lines of new/modified code across 10 files.**

---

## Rendering Order

The final render order in each frame is:

```
1. Shadow pass                     (existing)
2. glClear                         (existing)
3. RenderGrid()                    (NEW — depth write OFF, blend ON)
4. Render scene objects            (existing — depth write ON)
5. RenderGizmo()                   (NEW — depth test OFF, always on top)
6. ImGui overlay                   (existing)
   └─ Axis labels "X" "Y" "Z"     (NEW — ImGui foreground draw list)
```

---

## Visual Result

| Element | Color | Location |
|---------|-------|----------|
| Minor grid lines | Dark gray (0.25) | Every 1 unit on XZ plane |
| Major grid lines | Medium gray (0.4) | Every 10 units on XZ plane |
| X axis line (3D) | Red (1.0, 0.3, 0.3) | Along +X, 5 units |
| Y axis line (3D) | Green (0.3, 1.0, 0.3) | Along +Y, 5 units |
| Z axis line (3D) | Blue (0.3, 0.3, 1.0) | Along +Z, 5 units |
| "X" label | Red text | ImGui overlay near +X end |
| "Y" label | Green text | ImGui overlay near +Y end |
| "Z" label | Blue text | ImGui overlay near +Z end |
| Grid fade | Smooth alpha | Disappears beyond 80 units |

---

## Build and Test

```
PS E:\my_proj\ogle> .\build.bat Release
```

**Expected result:**
- Compiles without errors.
- A ground grid is visible on the XZ plane at Y=0.
- Grid has minor lines (1 unit) and major lines (10 units).
- Grid fades with distance from camera.
- Red/Blue tinted axis lines are visible on the ground plane (X and Z).
- Three colored lines extend from the origin (Red +X, Green +Y, Blue +Z).
- "X", "Y", "Z" labels are visible near the ends of the axis lines.
- "Show Grid" checkbox in the World window toggles the grid and gizmo on/off.
- Objects render on top of the grid (not occluded by it).
- Axis gizmo lines are always visible (not hidden behind objects).

**If you get compile errors:**

1. **`InitializeGrid` not declared:** Check that you added the method declaration in `OpenGLRenderer.h` private section.
2. **`LoadShaderSource` fails for grid.vs/grid.fs:** Check that the files exist at `assets/shaders/grid.vs` and `assets/shaders/grid.fs`. The path is relative to the working directory when the engine runs (usually the project root where `build.bat` is).
3. **`EditorState` has no member `showGrid`:** Check that you added `bool showGrid = true;` in `EditorState.h`.
4. **`Camera.h` not included in App.cpp:** Check that `#include "opengl/Camera.h"` exists. It should already be there from existing code.
5. **`fwidth` error in grid.fs:** Make sure the shader version is `#version 330 core`. The `fwidth` function is available in fragment shaders from GLSL 130+.

---

## How It Works

### Grid (Procedural Shader)
Instead of generating thousands of line vertices, we render a **single flat quad** (500x500 units). The fragment shader computes which pixel is on a grid line using `fwidth()` (screen-space derivative) for anti-aliased lines. This means:
- Zero geometry for the lines themselves
- Infinite resolution (lines are always sharp regardless of distance)
- Smooth fade at edges
- Very efficient (one draw call)

### Axis Gizmo (GL_LINES)
Six vertices forming three line segments from origin. Rendered with a trivial solid-color shader. Depth test is disabled so the gizmo is always visible.

### Labels (ImGui Overlay)
3D world positions are projected to 2D screen coordinates using the camera's view-projection matrix. The projected positions are used with `ImGui::GetForegroundDrawList()->AddText()` to draw colored labels.
