#include "ProceduralTexture.h"
#include "Texture2D.h"
#include "../opengl/ShaderManager.h"
#include "../Logger.h"
#include <glm/gtc/type_ptr.hpp>

namespace OGLE {

    std::string ProceduralTexture::GetComputeShaderName(ProceduralTextureType type) {
        switch (type) {
            case ProceduralTextureType::PerlinNoise:    return "perlin_noise";
            case ProceduralTextureType::FBM:            return "fbm_noise";
            case ProceduralTextureType::Marble:         return "marble";
            case ProceduralTextureType::Wood:           return "wood";
            case ProceduralTextureType::Clouds:         return "clouds";
            case ProceduralTextureType::Voronoi:        return "voronoi";
            case ProceduralTextureType::Checkerboard:   return "checkerboard";
            case ProceduralTextureType::RidgedNoise:    return "ridged_noise";
            case ProceduralTextureType::Turbulence:     return "turbulence";
            default:                                     return "perlin_noise";
        }
    }

    static const char* g_noiseHelper = R"(
float random(vec2 st, uint seed) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453123 + float(seed) * 0.001);
}
float perlin(vec2 st, uint seed) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    vec2 u = f * f * (3.0 - 2.0 * f);
    float n00 = random(i + vec2(0.0, 0.0), seed);
    float n10 = random(i + vec2(1.0, 0.0), seed);
    float n01 = random(i + vec2(0.0, 1.0), seed);
    float n11 = random(i + vec2(1.0, 1.0), seed);
    float nx0 = mix(n00, n10, u.x);
    float nx1 = mix(n01, n11, u.x);
    return mix(nx0, nx1, u.y);
}
float fbm(vec2 st, int octaves, float persistence, float lacunarity, uint seed) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * perlin(st * frequency, seed + uint(i));
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return value / maxValue;
}
float voronoi(vec2 st, uint seed) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float minDist = 1.0;
    for (float y = -1.0; y <= 1.0; y++) {
        for (float x = -1.0; x <= 1.0; x++) {
            vec2 neighbor = vec2(x, y);
            vec2 point = random(i + neighbor, seed) * vec2(0.5) + neighbor * 0.5;
            float dist = length(f - point);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}
)";

    bool ProceduralTexture::InitializeShaders() {
        ShaderManager* shaderManager = ShaderManager::GetGlobalInstance();
        if (!shaderManager) {
            LOG_ERROR("ShaderManager is null");
            return false;
        }

        // Здесь будут shader sources
        std::string noiseHelper(g_noiseHelper);

        // Загрузка всех compute shader'ов (упрощённо)
        bool success = true;

        // Для демонстрации - просто создадим dummy compute shaders
        const char* dummyCompute = "#version 430 core\nlayout(local_size_x=8,local_size_y=8)in;void main(){}";
        
        success &= shaderManager->loadComputeShader("perlin_noise", dummyCompute);
        success &= shaderManager->loadComputeShader("fbm_noise", dummyCompute);
        success &= shaderManager->loadComputeShader("marble", dummyCompute);
        success &= shaderManager->loadComputeShader("wood", dummyCompute);
        success &= shaderManager->loadComputeShader("clouds", dummyCompute);
        success &= shaderManager->loadComputeShader("voronoi", dummyCompute);
        success &= shaderManager->loadComputeShader("checkerboard", dummyCompute);
        success &= shaderManager->loadComputeShader("ridged_noise", dummyCompute);
        success &= shaderManager->loadComputeShader("turbulence", dummyCompute);

        // Связывание compute программ
        success &= shaderManager->linkComputeProgram("perlin_noise_program", "perlin_noise");
        success &= shaderManager->linkComputeProgram("fbm_noise_program", "fbm_noise");
        success &= shaderManager->linkComputeProgram("marble_program", "marble");
        success &= shaderManager->linkComputeProgram("wood_program", "wood");
        success &= shaderManager->linkComputeProgram("clouds_program", "clouds");
        success &= shaderManager->linkComputeProgram("voronoi_program", "voronoi");
        success &= shaderManager->linkComputeProgram("checkerboard_program", "checkerboard");
        success &= shaderManager->linkComputeProgram("ridged_noise_program", "ridged_noise");
        success &= shaderManager->linkComputeProgram("turbulence_program", "turbulence");

        return success;
    }

    GLuint ProceduralTexture::CreateTexture(int width, int height) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return texture;
    }

    bool ProceduralTexture::ExecuteComputeShader(
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
    ) {
        glUseProgram(program);

        GLint scaleUniform = glGetUniformLocation(program, "scale");
        if (scaleUniform != -1) glUniform1f(scaleUniform, scale);

        GLint octavesUniform = glGetUniformLocation(program, "octaves");
        if (octavesUniform != -1) glUniform1i(octavesUniform, octaves);

        GLint persistenceUniform = glGetUniformLocation(program, "persistence");
        if (persistenceUniform != -1) glUniform1f(persistenceUniform, persistence);

        GLint lacunarityUniform = glGetUniformLocation(program, "lacunarity");
        if (lacunarityUniform != -1) glUniform1f(lacunarityUniform, lacunarity);

        GLint color1Uniform = glGetUniformLocation(program, "color1");
        if (color1Uniform != -1) glUniform3fv(color1Uniform, 1, glm::value_ptr(color1));

        GLint color2Uniform = glGetUniformLocation(program, "color2");
        if (color2Uniform != -1) glUniform3fv(color2Uniform, 1, glm::value_ptr(color2));

        GLint seedUniform = glGetUniformLocation(program, "seed");
        if (seedUniform != -1) glUniform1ui(seedUniform, seed);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        int groupsX = (width + COMPUTE_SHADER_GROUP_SIZE - 1) / COMPUTE_SHADER_GROUP_SIZE;
        int groupsY = (height + COMPUTE_SHADER_GROUP_SIZE - 1) / COMPUTE_SHADER_GROUP_SIZE;
        glDispatchCompute(groupsX, groupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glUseProgram(0);

        return true;
    }

    std::shared_ptr<Texture2D> ProceduralTexture::Generate(
        ProceduralTextureType type,
        int width,
        int height,
        float scale,
        int octaves,
        float persistence,
        float lacunarity,
        const glm::vec3& color1,
        const glm::vec3& color2,
        unsigned int seed
    ) {
        ShaderManager* shaderManager = ShaderManager::GetGlobalInstance();
        if (!shaderManager) {
            LOG_ERROR("ShaderManager global instance not found");
            return nullptr;
        }

        std::string programName = GetComputeShaderName(type) + "_program";
        if (!shaderManager->hasProgram(programName)) {
            LOG_ERROR("Compute program not found: " + programName);
            return nullptr;
        }

        GLuint textureId = CreateTexture(width, height);
        if (textureId == 0) {
            LOG_ERROR("Failed to create texture");
            return nullptr;
        }

        GLuint program = shaderManager->getProgram(programName);
        if (!ExecuteComputeShader(
            program, textureId, width, height,
            scale, octaves, persistence, lacunarity,
            color1, color2, seed
        )) {
            glDeleteTextures(1, &textureId);
            LOG_ERROR("Failed to execute compute shader: " + programName);
            return nullptr;
        }

        LOG_INFO("Generated procedural texture: " + GetComputeShaderName(type));

        std::string textureName = "procedural_" + GetComputeShaderName(type) + "_" + std::to_string(seed);
        auto texture = Texture2D::CreateFromGLuint(textureId, width, height, textureName);
        
        if (!texture) {
            glDeleteTextures(1, &textureId);
            LOG_ERROR("Failed to create Texture2D from GLuint");
            return nullptr;
        }

        return texture;
    }

}
