#pragma once
#include <string>

class Layer
{
public:
    explicit Layer(const std::string& name = "Layer");
    virtual ~Layer() = default;

    // Called when the layer is pushed onto the stack
    virtual void OnAttach();

    // Called when the layer is popped from the stack
    virtual void OnDetach();

    // Called every frame for logic/update (physics, scripts, camera, etc.)
    virtual void OnUpdate(float deltaTime);

    // Called for custom OpenGL rendering (before ImGui)
    virtual void OnRender();

    // Called during ImGui frame for UI rendering
    virtual void OnImGuiRender();

    const std::string& GetName() const { return m_debugName; }

protected:
    std::string m_debugName;
};
