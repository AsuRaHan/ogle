// src/render/texture/TextureLoader.h
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

// Включаем заголовки stb - они будут доступны через CMake
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION  
#include "stb_image_write.h"

namespace ogle {

class TextureLoader {
public:
    // Загрузка изображения (8-bit)
    static bool LoadImage(const std::string& filepath,
                         std::vector<unsigned char>& outData,
                         int& outWidth, int& outHeight, int& outChannels,
                         bool flipVertical = true,
                         int desiredChannels = 0);
    
    // Загрузка HDR изображения (float)
    static bool LoadImageHDR(const std::string& filepath,
                            std::vector<float>& outData,
                            int& outWidth, int& outHeight, int& outChannels,
                            bool flipVertical = true);
    
    // Сохранение изображения
    static bool SaveImage(const std::string& filepath,
                         const unsigned char* data,
                         int width, int height, int channels);
    
    static bool SaveImageHDR(const std::string& filepath,
                            const float* data,
                            int width, int height, int channels);
    
    // Утилиты
    static bool FileExists(const std::string& filepath);
    static std::string GetExtension(const std::string& filepath);
};

} // namespace ogle