#pragma once

#include "render/Texture2D.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

struct EditorState;

class EditorProceduralTexturePanel
{
public:
    EditorProceduralTexturePanel();
    ~EditorProceduralTexturePanel();

    void Draw(EditorState& state);

private:
    std::shared_ptr<OGLE::Texture2D> m_generatedTexture;
    int m_selectedTextureType = 0;  // 0=PerlinNoise, 1=FBM, 2=Marble, 3=Wood, 4=Clouds, 5=Voronoi, 6=Checkerboard, 7=Mountains, 8=Turbulence
    int m_width = 512;
    int m_height = 512;
    float m_scale = 1.0f;
    int m_octaves = 4;
    float m_persistence = 0.5f;
    float m_lacunarity = 2.0f;
    float m_amplitude = 1.0f;
    unsigned int m_seed = 42;
    glm::vec3 m_color1 = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 m_color2 = glm::vec3(0.8f, 0.8f, 0.8f);
};
