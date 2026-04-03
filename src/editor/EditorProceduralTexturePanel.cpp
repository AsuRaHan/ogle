#include "editor/EditorProceduralTexturePanel.h"

#include "editor/EditorState.h"
#include "render/TextureGenerator.h"
#include "Logger.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

EditorProceduralTexturePanel::EditorProceduralTexturePanel()
    : m_generatedTexture(nullptr)
{
}

EditorProceduralTexturePanel::~EditorProceduralTexturePanel()
{
    m_generatedTexture.reset();
}

void EditorProceduralTexturePanel::Draw(EditorState& state)
{
    ImGui::TextUnformatted("Procedural Texture Generator");
    ImGui::Separator();

    // Texture type selection
    const char* textureTypes[] = {
        "Perlin Noise",
        "FBM (Fractional Brownian Motion)",
        "Marble",
        "Wood",
        "Clouds",
        "Voronoi",
        "Checkerboard",
        "Mountains",
        "Turbulence"
    };
    ImGui::Combo("Texture Type", &m_selectedTextureType, textureTypes, IM_ARRAYSIZE(textureTypes));

    // Dimensions
    ImGui::SliderInt("Width##ptex", &m_width, 64, 2048);
    ImGui::SliderInt("Height##ptex", &m_height, 64, 2048);

    // Parameters
    ImGui::SliderFloat("Scale", &m_scale, 0.1f, 10.0f);
    ImGui::SliderInt("Octaves", &m_octaves, 1, 8);
    ImGui::SliderFloat("Persistence", &m_persistence, 0.0f, 1.0f);
    ImGui::SliderFloat("Lacunarity", &m_lacunarity, 1.0f, 4.0f);
    ImGui::SliderFloat("Amplitude", &m_amplitude, 0.1f, 2.0f);
    ImGui::InputInt("Seed", reinterpret_cast<int*>(&m_seed));

    // Color parameters
    ImGui::ColorEdit3("Color 1##ptex", glm::value_ptr(m_color1));
    ImGui::ColorEdit3("Color 2##ptex", glm::value_ptr(m_color2));

    ImGui::Separator();

    // Generate button
    if (ImGui::Button("Generate Texture", ImVec2(-1.0f, 0.0f))) {
        try {
            switch (m_selectedTextureType) {
                case 0:  // Perlin Noise
                    m_generatedTexture = OGLE::TextureGenerator::GeneratePerlinNoise(
                        m_width, m_height, m_scale, m_color1, m_color2, m_seed);
                    break;
                case 1:  // FBM
                    m_generatedTexture = OGLE::TextureGenerator::GenerateFBM(
                        m_width, m_height, m_scale, m_octaves, m_persistence, m_lacunarity, m_color1, m_color2, m_seed);
                    break;
                case 2:  // Marble
                    m_generatedTexture = OGLE::TextureGenerator::GenerateMarble(
                        m_width, m_height, m_scale, m_octaves, m_color1, m_color2, m_seed);
                    break;
                case 3:  // Wood
                    m_generatedTexture = OGLE::TextureGenerator::GenerateWood(
                        m_width, m_height, m_scale, m_octaves, m_color1, m_color2, m_seed);
                    break;
                case 4:  // Clouds
                    m_generatedTexture = OGLE::TextureGenerator::GenerateClouds(
                        m_width, m_height, m_scale, m_octaves, m_persistence, m_lacunarity, m_seed);
                    break;
                case 5:  // Voronoi
                    m_generatedTexture = OGLE::TextureGenerator::GenerateVoronoi(
                        m_width, m_height, m_scale, m_color1, m_color2, m_seed);
                    break;
                case 6:  // Checkerboard
                    m_generatedTexture = OGLE::TextureGenerator::GenerateCheckerboard(
                        m_width, m_height, m_scale, m_color1, m_color2, m_seed);
                    break;
                case 7:  // Mountains (Ridged Noise)
                    m_generatedTexture = OGLE::TextureGenerator::GenerateMountains(
                        m_width, m_height, m_scale, m_octaves, m_color1, m_color2, m_seed);
                    break;
                case 8:  // Turbulence
                    m_generatedTexture = OGLE::TextureGenerator::GenerateTurbulence(
                        m_width, m_height, m_scale, m_octaves, m_color1, m_color2, m_seed);
                    break;
                default:
                    break;
            }

            if (m_generatedTexture) {
                LOG_INFO("Procedural texture generated: " + std::to_string(m_width) + "x" + std::to_string(m_height));
            }
        } catch (const std::exception& ex) {
            LOG_ERROR("Failed to generate texture: " + std::string(ex.what()));
        }
    }

    // Preview
    if (m_generatedTexture && m_generatedTexture->GetTextureId() != 0) {
        ImGui::Separator();
        ImGui::Text("Preview:");
        ImVec2 previewSize(256.0f, 256.0f);
        ImGui::Image(
            reinterpret_cast<void*>(static_cast<intptr_t>(m_generatedTexture->GetTextureId())),
            previewSize,
            ImVec2(0, 1),
            ImVec2(1, 0));

        ImGui::TextDisabled("Export feature requires additional image library support");
    } else {
        ImGui::TextDisabled("Generate a texture to see preview");
    }
}
