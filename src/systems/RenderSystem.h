// src/systems/RenderSystem.h
#pragma once

#include "core/ISystem.h"
#include "log/Logger.h"
#include "window/OpenGLContext.h" 

#include <glad/gl.h>
#include <string>
#include <memory>

namespace ogle {

class RenderSystem final : public ISystem {
public:
    explicit RenderSystem(HDC hdc);
    ~RenderSystem() override;

    const std::string& GetName() const override;

    bool Initialize() override; 
    void Update(float deltaTime) override {}
    void Render() override;
    void Shutdown() override;

    void OnResize(int width, int height) override;

    // Для настройки
    void SetClearColor(float r, float g, float b, float a = 1.0f);

private:
    HDC m_hdc;  // просто храним, что передали
    std::unique_ptr<OpenGLContext> m_context;

    float m_clearColor[4] = {0.1f, 0.1f, 0.3f, 1.0f};
};

} // namespace ogle