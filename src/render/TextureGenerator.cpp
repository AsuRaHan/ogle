#include "TextureGenerator.h"
#include "Texture2D.h"

namespace OGLE {

    std::shared_ptr<Texture2D> TextureGenerator::GenerateClouds(
        int width, int height, float scale, int octaves, float persistence, float lacunarity, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::Clouds,
            width, height, scale, octaves, persistence, lacunarity,
            glm::vec3(0.3f, 0.3f, 0.3f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateMarble(
        int width, int height, float scale, int octaves, const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::Marble,
            width, height, scale, octaves, 0.5f, 2.0f,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateWood(
        int width, int height, float scale, int octaves, const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::Wood,
            width, height, scale, octaves, 0.5f, 2.0f,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateMountains(
        int width, int height, float scale, int octaves, const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::RidgedNoise,
            width, height, scale, octaves, 0.5f, 2.0f,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateCheckerboard(
        int width, int height, float checkerCount, const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::Checkerboard,
            width, height, checkerCount, 1, 0.5f, 2.0f,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateVoronoi(
        int width, int height, float scale, const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::Voronoi,
            width, height, scale, 1, 0.5f, 2.0f,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GeneratePerlinNoise(
        int width, int height, float scale, const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::PerlinNoise,
            width, height, scale, 1, 0.5f, 2.0f,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateFBM(
        int width, int height, float scale, int octaves, float persistence, float lacunarity,
        const glm::vec3& color1, const glm::vec3& color2, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::FBM,
            width, height, scale, octaves, persistence, lacunarity,
            color1, color2, seed
        );
    }

    std::shared_ptr<Texture2D> TextureGenerator::GenerateTurbulence(
        int width, int height, float scale, int octaves, const glm::vec3& colorCold, const glm::vec3& colorHot, unsigned int seed)
    {
        return ProceduralTexture::Generate(
            ProceduralTextureType::Turbulence,
            width, height, scale, octaves, 0.5f, 2.0f,
            colorCold, colorHot, seed
        );
    }

}
