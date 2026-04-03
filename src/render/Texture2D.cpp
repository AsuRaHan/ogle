#include "render/Texture2D.h"

#include "Logger.h"
#include "core/FileSystem.h"

#include <wincodec.h>

#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace OGLE {
    namespace
    {
        std::mutex g_textureCacheMutex;
        std::unordered_map<std::string, std::weak_ptr<Texture2D>> g_textureCache;

        bool EnsureComInitialized()
        {
            const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            return SUCCEEDED(result) || result == RPC_E_CHANGED_MODE || result == S_FALSE;
        }
    }

    Texture2D::~Texture2D()
    {
        if (m_textureId != 0) {
            glDeleteTextures(1, &m_textureId);
        }
    }

    std::shared_ptr<Texture2D> Texture2D::LoadShared(const std::string& path)
    {
        const std::string resolvedPath = FileSystem::ResolvePath(path).string();

        {
            std::lock_guard<std::mutex> lock(g_textureCacheMutex);
            const auto cacheIt = g_textureCache.find(resolvedPath);
            if (cacheIt != g_textureCache.end()) {
                if (std::shared_ptr<Texture2D> cachedTexture = cacheIt->second.lock()) {
                    return cachedTexture;
                }
            }
        }

        std::shared_ptr<Texture2D> texture(new Texture2D());
        if (!texture->LoadFromFile(resolvedPath)) {
            return {};
        }

        {
            std::lock_guard<std::mutex> lock(g_textureCacheMutex);
            g_textureCache[resolvedPath] = texture;
        }

        return texture;
    }

    bool Texture2D::LoadFromFile(const std::string& path)
    {
        if (!EnsureComInitialized()) {
            LOG_ERROR("Failed to initialize COM for texture loading");
            return false;
        }

        IWICImagingFactory* factory = nullptr;
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory));
        if (FAILED(hr) || !factory) {
            LOG_ERROR("Failed to create WIC factory for texture loading");
            return false;
        }

        IWICBitmapDecoder* decoder = nullptr;
        const std::wstring widePath = std::filesystem::path(path).wstring();
        hr = factory->CreateDecoderFromFilename(
            widePath.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            &decoder);
        if (FAILED(hr) || !decoder) {
            factory->Release();
            LOG_ERROR("Failed to open texture file: " + path);
            return false;
        }

        IWICBitmapFrameDecode* frame = nullptr;
        hr = decoder->GetFrame(0, &frame);
        if (FAILED(hr) || !frame) {
            decoder->Release();
            factory->Release();
            LOG_ERROR("Failed to read texture frame: " + path);
            return false;
        }

        IWICFormatConverter* converter = nullptr;
        hr = factory->CreateFormatConverter(&converter);
        if (FAILED(hr) || !converter) {
            frame->Release();
            decoder->Release();
            factory->Release();
            LOG_ERROR("Failed to create WIC converter for texture: " + path);
            return false;
        }

        hr = converter->Initialize(
            frame,
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0f,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) {
            converter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            LOG_ERROR("Failed to convert texture pixels to RGBA: " + path);
            return false;
        }

        UINT width = 0;
        UINT height = 0;
        converter->GetSize(&width, &height);
        if (width == 0 || height == 0) {
            converter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            LOG_ERROR("Texture has invalid dimensions: " + path);
            return false;
        }

        std::vector<unsigned char> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
        hr = converter->CopyPixels(
            nullptr,
            width * 4,
            static_cast<UINT>(pixels.size()),
            pixels.data());

        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();

        if (FAILED(hr)) {
            LOG_ERROR("Failed to copy texture pixels: " + path);
            return false;
        }

        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_width = static_cast<int>(width);
        m_height = static_cast<int>(height);
        m_path = path;

        LOG_INFO("Loaded texture: " + path);
        return true;
    }

    std::shared_ptr<Texture2D> Texture2D::CreateFromGLuint(GLuint textureId, int width, int height, const std::string& name)
    {
        if (textureId == 0 || width <= 0 || height <= 0) {
            LOG_ERROR("Invalid parameters for CreateFromGLuint: textureId=" + std::to_string(textureId) 
                    + ", width=" + std::to_string(width) + ", height=" + std::to_string(height));
            return nullptr;
        }

        std::shared_ptr<Texture2D> texture(new Texture2D());
        texture->m_textureId = textureId;
        texture->m_width = width;
        texture->m_height = height;
        texture->m_path = name;

        LOG_INFO("Created texture from GLuint: " + name + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
        return texture;
    }
}
