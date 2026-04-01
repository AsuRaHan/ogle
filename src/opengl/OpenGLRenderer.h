#pragma once
#ifndef OPENGLRENDERER_H_
#define OPENGLRENDERER_H_

#include "GLFunctions.h"
#include "ShaderManager.h"
#include "DomoScene.h"
#include <memory>
// Needed for std::chrono::steady_clock used in the implementation
#include <chrono>

namespace ogle {
    class Camera;
}

namespace OGLE {
    class World;
}

class OpenGLRenderer {
public:
    OpenGLRenderer(int width, int height, ogle::Camera& camera, OGLE::World& world);
    ~OpenGLRenderer();

    bool Initialize();
    void Render();
    void Resize(int width, int height);

private:
    ShaderManager m_shaderManager;
    ogle::Camera& m_camera;
    OGLE::World& m_world;
    int m_width;
    int m_height;

    std::unique_ptr<DomoScene> m_scene;
    // Time point marking when the renderer was created, used for delta time calculation
    std::chrono::steady_clock::time_point m_startTime;
};

#endif // OPENGLRENDERER_H_
