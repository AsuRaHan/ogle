#pragma once
#include "core/Layer.h"
#include <memory>

class CameraManager;
class WorldManager;
// class Editor;

class ExampleLayer : public Layer
{
public:
    ExampleLayer();
    ~ExampleLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;

private:
    // Здесь можно хранить указатели на нужные системы
    // (лучше получать через App, но для примера можно кэшировать)
    CameraManager* m_cameraManager = nullptr;
    WorldManager* m_worldManager = nullptr;

    bool m_showDemoWindow = true;
    float m_exampleValue = 0.0f;
};
