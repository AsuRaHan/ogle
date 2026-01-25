// src/render/TextureController.cpp (упрощенная версия)
#include "TextureController.h"
#include "render/texture/TextureControllerImpl.h"

namespace ogle {

TextureController& TextureController::Get() {
    static TextureController instance;
    return instance;
}

TextureController::TextureController()
    : m_impl(std::make_unique<TextureControllerImpl>()) {
    Logger::Info("TextureController initialized");
}

TextureController::~TextureController() {
    Logger::Info("TextureController shutdown");
}

// Делегируем все вызовы
Texture* TextureController::CreateTexture2D(const std::string& name, 
                                           int width, int height,
                                           const std::string& filepath) {
    return m_impl->CreateTexture2D(name, width, height, filepath);
}

Texture* TextureController::CreateTextureCube(const std::string& name,
                                             const std::vector<std::string>& facePaths) {
    return m_impl->CreateTextureCube(name, facePaths);
}

Texture* TextureController::LoadTexture2D(const std::string& filepath, 
                                         const std::string& textureName) {
    return m_impl->LoadTexture2D(filepath, textureName);
}

Texture* TextureController::LoadTextureCube(const std::string& filepath,
                                           const std::string& textureName) {
    return m_impl->LoadTextureCube(filepath, textureName);
}

Texture* TextureController::GetTexture(const std::string& name) {
    return m_impl->GetTexture(name);
}

bool TextureController::HasTexture(const std::string& name) const {
    return m_impl->HasTexture(name);
}

void TextureController::RemoveTexture(const std::string& name) {
    m_impl->RemoveTexture(name);
}

Texture* TextureController::GetBuiltin(Builtin type) {
    return m_impl->GetBuiltin(static_cast<BuiltinTexture>(type));
}

void TextureController::PreloadBuiltinTextures() {
    m_impl->PreloadBuiltinTextures();
}

void TextureController::Clear() {
    m_impl->Clear();
}

size_t TextureController::GetTextureCount() const {
    return m_impl->GetTextureCount();
}

std::vector<std::string> TextureController::GetTextureNames() const {
    return m_impl->GetTextureNames();
}

std::vector<Texture*> TextureController::GetTexturesByType(TextureType type) const {
    return m_impl->GetTexturesByType(type);
}

void TextureController::PrintDebugInfo() const {
    m_impl->PrintDebugInfo();
}

void TextureController::WatchTextureFiles(bool enable) {
    m_impl->WatchTextureFiles(enable);
}

void TextureController::CheckForUpdates() {
    m_impl->CheckForUpdates();
}

} // namespace ogle