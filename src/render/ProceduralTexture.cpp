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

        // Helper to build and load a compute shader
        auto loadAndLink = [&](const std::string& name, const std::string& source) {
            if (!shaderManager->loadComputeShader(name, source.c_str())) {
                LOG_ERROR("Failed to load compute shader: " + name);
                return false;
            }
            if (!shaderManager->linkComputeProgram(name + "_program", name)) {
                LOG_ERROR("Failed to link compute program: " + name + "_program");
                return false;
            }
            return true;
        };

        bool success = true;

        // --- Shader Sources ---
        const std::string header = R"(
#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform writeonly image2D destTex;

uniform float scale;
uniform int octaves;
uniform float persistence;
uniform float lacunarity;
uniform vec3 color1;
uniform vec3 color2;
uniform uint seed;
)";

        auto getMain = [](const std::string& body) {
            return R"(
void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destTex);
    if (pixel_coords.x >= size.x || pixel_coords.y >= size.y) {
        return;
    }
    vec2 uv = vec2(pixel_coords) / vec2(size);
    vec3 finalColor;
    
    )" + body + R"(

    imageStore(destTex, pixel_coords, vec4(finalColor, 1.0));
}
)";
        };

        // --- Actual Implementations ---

        // Checkerboard
        std::string checkerboardBody = R"(
    float valX = floor(uv.x * scale);
    float valY = floor(uv.y * scale);
    finalColor = (mod(valX + valY, 2.0) == 0.0) ? color1 : color2;)";
        success &= loadAndLink("checkerboard", header + getMain(checkerboardBody));

        // Perlin Noise
        std::string perlinBody = R"(
    float n = perlin(uv * scale, seed);
    finalColor = mix(color1, color2, n);)";
        success &= loadAndLink("perlin_noise", header + g_noiseHelper + getMain(perlinBody));

        // FBM Noise
        std::string fbmBody = R"(
    float n = fbm(uv * scale, octaves, persistence, lacunarity, seed);
    finalColor = mix(color1, color2, n);)";
        std::string fbmSrc = header + g_noiseHelper + getMain(fbmBody);
        success &= loadAndLink("fbm_noise", fbmSrc);
        success &= loadAndLink("clouds", fbmSrc); // Clouds can be FBM

        // Dummy shader for unimplemented types
        std::string dummyBody = "finalColor = vec3(1.0, 0.0, 1.0); // Magenta for unimplemented shaders";
        std::string dummySrc = header + getMain(dummyBody);
        success &= loadAndLink("marble", dummySrc);
        success &= loadAndLink("wood", dummySrc);
        success &= loadAndLink("voronoi", dummySrc);
        success &= loadAndLink("ridged_noise", dummySrc);
        success &= loadAndLink("turbulence", dummySrc);

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
