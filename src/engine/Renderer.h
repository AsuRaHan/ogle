#pragma once
#include <glad/gl.h>
#include "window/MainWindow.h"

class Scene;
class Camera;

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Render(const Scene& scene, const Camera& camera);
};