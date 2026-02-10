// src/systems/RenderSystem.h
#pragma once

#include <glad/gl.h>
#include <memory>
#include <vector>
#include <functional>
#include "core/ISystem.h"
#include "log/Logger.h"
#include "window/OpenGLContext.h"
#include "test/TestCube.h"
#include "render/Camera.h"
#include "systems/GuiSystem.h"
#include "scene/Scene.h"

//#include "ui/UIController.h"

namespace ogle {

class RenderSystem final : public ISystem {
public:
    explicit RenderSystem(HDC hdc);
    ~RenderSystem() override;

    const std::string& GetName() const override {
        static std::string n = "RenderSystem";
        return n;
    }

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    void OnResize(int width, int height) override;
    //void SetClearColor(float r, float g, float b, float a = 1.0f);

    // === Управление рендерерами ===
    void AddRenderer(ISystem* system);
    void RemoveRenderer(ISystem* system);
    void ClearRenderers();
    
    // === GUI ===
    void SetGuiSystem(GuiSystem* guiSystem);
    void EnableGUI(bool enable) { m_guiEnabled = enable; }

private:
    HDC m_hdc;
    std::unique_ptr<OpenGLContext> m_context;
    float m_clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };

    TestCube m_testCube;
    float m_time = 0.0f;
    
    Camera* m_camera = nullptr;

    Scene* m_scene = nullptr;

    // Список рендереров
    std::vector<ISystem*> m_renderers;
    
    // GUI система
    GuiSystem* m_guiSystem = nullptr;
    bool m_guiEnabled = true;
};

} // namespace ogle