#pragma once

#include "ProceduralTexture.h"
#include <memory>
#include <glm/vec3.hpp>

namespace OGLE {
    class Texture2D;

    // Удобный класс для генерации различных типов текстур
    class TextureGenerator {
    public:
        // Облака для неба/атмосферы
        static std::shared_ptr<Texture2D> GenerateClouds(
            int width = 512,
            int height = 512,
            float scale = 2.0f,
            int octaves = 6,
            float persistence = 0.5f,
            float lacunarity = 2.0f,
            unsigned int seed = 42
        );

        // Мраморная текстура
        static std::shared_ptr<Texture2D> GenerateMarble(
            int width = 512,
            int height = 512,
            float scale = 2.0f,
            int octaves = 4,
            const glm::vec3& color1 = glm::vec3(0.2f, 0.2f, 0.2f),
            const glm::vec3& color2 = glm::vec3(0.9f, 0.9f, 0.9f),
            unsigned int seed = 42
        );

        // Текстура дерева
        static std::shared_ptr<Texture2D> GenerateWood(
            int width = 512,
            int height = 512,
            float scale = 1.5f,
            int octaves = 3,
            const glm::vec3& color1 = glm::vec3(0.3f, 0.2f, 0.1f),
            const glm::vec3& color2 = glm::vec3(0.6f, 0.4f, 0.2f),
            unsigned int seed = 42
        );

        // Текстура гор (ридж шум)
        static std::shared_ptr<Texture2D> GenerateMountains(
            int width = 512,
            int height = 512,
            float scale = 3.0f,
            int octaves = 8,
            const glm::vec3& color1 = glm::vec3(0.1f, 0.2f, 0.3f),
            const glm::vec3& color2 = glm::vec3(0.9f, 0.9f, 0.9f),
            unsigned int seed = 42
        );

        // Шахматная текстура
        static std::shared_ptr<Texture2D> GenerateCheckerboard(
            int width = 512,
            int height = 512,
            float squareSize = 32.0f,
            const glm::vec3& color1 = glm::vec3(0.0f),
            const glm::vec3& color2 = glm::vec3(1.0f),
            unsigned int seed = 42
        );

        // Диаграмма Вороного
        static std::shared_ptr<Texture2D> GenerateVoronoi(
            int width = 512,
            int height = 512,
            float scale = 5.0f,
            const glm::vec3& color1 = glm::vec3(0.2f, 0.2f, 0.2f),
            const glm::vec3& color2 = glm::vec3(0.8f, 0.8f, 0.8f),
            unsigned int seed = 42
        );

        // Перлин шум (базовый)
        static std::shared_ptr<Texture2D> GeneratePerlinNoise(
            int width = 512,
            int height = 512,
            float scale = 2.0f,
            const glm::vec3& color1 = glm::vec3(0.0f),
            const glm::vec3& color2 = glm::vec3(1.0f),
            unsigned int seed = 42
        );

        // FBM шум (дополнительный контроль)
        static std::shared_ptr<Texture2D> GenerateFBM(
            int width = 512,
            int height = 512,
            float scale = 2.0f,
            int octaves = 4,
            float persistence = 0.5f,
            float lacunarity = 2.0f,
            const glm::vec3& color1 = glm::vec3(0.0f),
            const glm::vec3& color2 = glm::vec3(1.0f),
            unsigned int seed = 42
        );

        // Турбулентность (для огня, дыма)
        static std::shared_ptr<Texture2D> GenerateTurbulence(
            int width = 512,
            int height = 512,
            float scale = 2.0f,
            int octaves = 4,
            const glm::vec3& colorCold = glm::vec3(0.0f, 0.0f, 1.0f),
            const glm::vec3& colorHot = glm::vec3(1.0f, 0.0f, 0.0f),
            unsigned int seed = 42
        );
    };
}
