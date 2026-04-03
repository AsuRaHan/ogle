#pragma once

#include "../opengl/GLFunctions.h"
#include <memory>
#include <string>
#include <glm/vec3.hpp>

// Forward declaration
class ShaderManager;

namespace OGLE {
    class Texture2D;

    // Типы процедурных текстур
    enum class ProceduralTextureType {
        PerlinNoise,        // 2D Перлин шум
        FBM,                // Fractional Brownian Motion
        Marble,             // Мраморный паттерн
        Wood,               // Текстура дерева
        Clouds,             // Облака
        Voronoi,            // Диаграмма Вороного
        Checkerboard,       // Шахматная доска
        RidgedNoise,        // Ридж шум (горы)
        Turbulence          // Турбулентность
    };

    class ProceduralTexture {
    public:
        // Создание процедурной текстуры с использованием compute shader'а
        static std::shared_ptr<Texture2D> Generate(
            ProceduralTextureType type,
            int width = 512,
            int height = 512,
            float scale = 1.0f,
            int octaves = 4,
            float persistence = 0.5f,
            float lacunarity = 2.0f,
            const glm::vec3& color1 = glm::vec3(0.0f),
            const glm::vec3& color2 = glm::vec3(1.0f),
            unsigned int seed = 12345u
        );

        // Получить имя компьютерного shader'а по типу текстуры
        static std::string GetComputeShaderName(ProceduralTextureType type);

        // Инициализация compute shader'ов для генерации
        static bool InitializeShaders();

    private:
        // Вспомогательные методы для генерации
        static GLuint CreateTexture(int width, int height);
        static bool ExecuteComputeShader(
            GLuint program,
            GLuint texture,
            int width,
            int height,
            float scale,
            int octaves,
            float persistence,
            float lacunarity,
            const glm::vec3& color1,
            const glm::vec3& color2,
            unsigned int seed
        );

        static constexpr int COMPUTE_SHADER_GROUP_SIZE = 8;
    };
}
