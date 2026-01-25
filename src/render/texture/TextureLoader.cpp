// src/render/texture/TextureLoader.cpp
#include "TextureLoader.h"
#include "log/Logger.h"
#include <filesystem>

namespace ogle {

bool TextureLoader::LoadImage(const std::string& filepath,
                             std::vector<unsigned char>& outData,
                             int& outWidth, int& outHeight, int& outChannels,
                             bool flipVertical,
                             int desiredChannels) {
    
    if (!FileExists(filepath)) {
        Logger::Error("Texture file not found: " + filepath);
        return false;
    }
    
    // STB загружает с началом координат вверху, OpenGL ожидает внизу
    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);
    
    unsigned char* data = stbi_load(filepath.c_str(),
                                   &outWidth, &outHeight, &outChannels,
                                   desiredChannels);
    
    if (!data) {
        Logger::Error("Failed to load texture: " + filepath + 
                     " - " + stbi_failure_reason());
        return false;
    }
    
    if (desiredChannels > 0) {
        outChannels = desiredChannels;
    }
    
    // Копируем данные в вектор
    size_t dataSize = outWidth * outHeight * outChannels;
    outData.resize(dataSize);
    memcpy(outData.data(), data, dataSize);
    
    stbi_image_free(data);
    
    Logger::Debug("Texture loaded: " + filepath + 
                 " (" + std::to_string(outWidth) + "x" + 
                 std::to_string(outHeight) + ", channels: " + 
                 std::to_string(outChannels) + ")");
    
    return true;
}

bool TextureLoader::LoadImageHDR(const std::string& filepath,
                                std::vector<float>& outData,
                                int& outWidth, int& outHeight, int& outChannels,
                                bool flipVertical) {
    
    if (!FileExists(filepath)) {
        Logger::Error("HDR texture file not found: " + filepath);
        return false;
    }
    
    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);
    
    float* data = stbi_loadf(filepath.c_str(),
                            &outWidth, &outHeight, &outChannels,
                            0); // Все каналы
    
    if (!data) {
        Logger::Error("Failed to load HDR texture: " + filepath);
        return false;
    }
    
    size_t dataSize = outWidth * outHeight * outChannels;
    outData.resize(dataSize);
    memcpy(outData.data(), data, dataSize * sizeof(float));
    
    stbi_image_free(data);
    
    Logger::Debug("HDR texture loaded: " + filepath);
    return true;
}

bool TextureLoader::SaveImage(const std::string& filepath,
                             const unsigned char* data,
                             int width, int height, int channels) {
    
    std::string ext = GetExtension(filepath);
    
    if (ext == "png") {
        return stbi_write_png(filepath.c_str(), width, height, channels, data, width * channels) != 0;
    } else if (ext == "jpg" || ext == "jpeg") {
        return stbi_write_jpg(filepath.c_str(), width, height, channels, data, 90) != 0;
    } else if (ext == "bmp") {
        return stbi_write_bmp(filepath.c_str(), width, height, channels, data) != 0;
    } else if (ext == "tga") {
        return stbi_write_tga(filepath.c_str(), width, height, channels, data) != 0;
    }
    
    Logger::Error("Unsupported image format: " + ext);
    return false;
}

bool TextureLoader::SaveImageHDR(const std::string& filepath,
                                const float* data,
                                int width, int height, int channels) {
    return stbi_write_hdr(filepath.c_str(), width, height, channels, data) != 0;
}

bool TextureLoader::FileExists(const std::string& filepath) {
    return std::filesystem::exists(filepath);
}

std::string TextureLoader::GetExtension(const std::string& filepath) {
    size_t pos = filepath.find_last_of('.');
    if (pos != std::string::npos) {
        std::string ext = filepath.substr(pos + 1);
        // Приводим к нижнему регистру
        for (char& c : ext) c = tolower(c);
        return ext;
    }
    return "";
}

} // namespace ogle