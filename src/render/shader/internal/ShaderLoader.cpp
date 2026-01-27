// src/render/shader/internal/ShaderLoader.cpp
#include "ShaderLoader.h"

namespace ogle {

	ShaderLoader::ShaderLoader() {
		Logger::Info("ShaderLoader initialized");
	}

	std::string ShaderLoader::LoadFromFile(const std::string& filepath) {
		// Проверяем кэш
		auto it = m_cache.find(filepath);
		if (it != m_cache.end()) {
			auto currentTime = std::filesystem::last_write_time(filepath);
			if (currentTime == it->second.lastWrite) {
				return it->second.content; // Кэш актуален
			}
		}

		// Загружаем файл
		std::string content = ReadFile(filepath);
		if (content.empty()) {
			Logger::Error("Failed to load shader: " + filepath);
			return "";
		}

		// Кэшируем
		m_cache[filepath] = { content, std::filesystem::last_write_time(filepath) };

		Logger::Debug("Shader loaded: " + filepath);
		return content;
	}


    std::string ShaderLoader::GetBuiltinSource(const std::string& shaderName) {
        // ==================== BASIC COLOR ====================
        if (shaderName == "BasicColor.vert") {
            return R"(
#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;      // ← ВЫХОДИТ из вершинного шейдера
out vec2 vTexCoord;    // ← ВЫХОДИТ из вершинного шейдера

void main() {
    vPosition = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";
        }

        if (shaderName == "BasicColor.frag") {
            return R"(
#version 460 core
in vec3 vPosition;     // ← ЕСТЬ
in vec3 vNormal;       // ← ЕСТЬ
in vec2 vTexCoord;     // ← ЕСТЬ

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform bool uUseLighting;
uniform bool uUseTexture;
uniform sampler2D uTexture;

out vec4 FragColor;    // ← ЕСТЬ

void main() {
    vec3 color = uColor;
    
    // Если есть текстура - используем её
    if (uUseTexture) {
        color = texture(uTexture, vTexCoord).rgb;
    }
    
    if (uUseLighting) {
        float light = max(dot(normalize(vNormal), normalize(uLightDir)), 0.2);
        color *= light;
    }
    
    FragColor = vec4(color, 1.0);
}
)";
        }

        // ==================== BASIC TEXTURE ====================
        if (shaderName == "BasicTexture.vert") {
            return R"(
#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vPosition = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";
        }

        if (shaderName == "BasicTexture.frag") {
            return R"(
#version 460 core
in vec3 vPosition;          // ← ЭТО ДОЛЖНО БЫТЬ
in vec3 vNormal;            // ← ЭТО ДОЛЖНО БЫТЬ  
in vec2 vTexCoord;          // ← ЭТО ДОЛЖНО БЫТЬ

uniform sampler2D uTexture;
uniform vec3 uLightDir;
uniform bool uUseLighting;
uniform vec3 uColor;
uniform bool uUseColor;

out vec4 FragColor;         // ← ЭТО ДОЛЖНО БЫТЬ

void main() {
    vec3 color;
    
    if (uUseColor) {
        color = uColor;
    } else {
        color = texture(uTexture, vTexCoord).rgb;  // ← использует vTexCoord
    }
    
    if (uUseLighting) {
        float light = max(dot(normalize(vNormal), normalize(uLightDir)), 0.2);  // ← использует vNormal
        color *= light;
    }
    
    FragColor = vec4(color, 1.0);
}
)";
        }

        // ==================== SKYBOX ====================
        if (shaderName == "Skybox.vert") {
            return R"(#version 460 core
layout(location = 0) in vec3 aPosition;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vTexCoord;

void main() {
    vTexCoord = aPosition;
    vec4 pos = uProjection * mat4(mat3(uView)) * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;
})";
        }

        if (shaderName == "Skybox.frag") {
            return R"(#version 460 core
in vec3 vTexCoord;

uniform samplerCube uSkybox;

out vec4 FragColor;

void main() {
    FragColor = texture(uSkybox, vTexCoord);
})";
        }

        // ==================== UNLIT ====================
        if (shaderName == "Unlit.vert") {
            return R"(#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vPosition = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
})";
        }

        if (shaderName == "Unlit.frag") {
            return R"(#version 460 core
in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture) {
        FragColor = texture(uTexture, vTexCoord);
    } else {
        FragColor = vec4(uColor, 1.0);
    }
})";
        }

        // ==================== WIREFRAME ====================
        if (shaderName == "Wireframe.vert") {
            return R"(#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;

void main() {
    vPosition = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
})";
        }

        if (shaderName == "Wireframe.frag") {
            return R"(#version 460 core
in vec3 vPosition;
in vec3 vNormal;

uniform vec3 uColor;
uniform float uLineWidth = 1.0;
uniform bool uShowNormals;

out vec4 FragColor;

void main() {
    if (uShowNormals) {
        // Показываем нормали как цвет (отладочный режим)
        FragColor = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
    } else {
        // Обычный цвет для линий
        FragColor = vec4(uColor, 1.0);
    }
})";
        }

        // ==================== DEBUG NORMALS ====================
        if (shaderName == "DebugNormals.vert") {
            return R"(#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uNormalScale = 0.1;

out vec3 vNormal;
out vec3 vWorldPos;

void main() {
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vNormal = normalize(normalMatrix * aNormal);
    vWorldPos = vec3(uModel * vec4(aPosition, 1.0));
    
    // Смещаем позицию по нормали для визуализации
    vec3 posWithNormal = vWorldPos + vNormal * uNormalScale;
    gl_Position = uProjection * uView * vec4(posWithNormal, 1.0);
})";
        }

        if (shaderName == "DebugNormals.frag") {
            return R"(#version 460 core
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uColor = vec3(1.0, 0.0, 0.0);

out vec4 FragColor;

void main() {
    // Красный для положительных нормалей, синий для отрицательных
    FragColor = vec4(vNormal * 0.5 + 0.5, 1.0);
})";
        }

        Logger::Warning("Builtin shader not found: " + shaderName);
        return "";
    }

	std::string ShaderLoader::Preprocess(const std::string& source,
		const std::string& basePath) {
		return ResolveIncludes(source, basePath);
	}

	bool ShaderLoader::HasFileChanged(const std::string& filepath) {
		auto it = m_cache.find(filepath);
		if (it == m_cache.end()) return true;

		try {
			auto currentTime = std::filesystem::last_write_time(filepath);
			return currentTime != it->second.lastWrite;
		}
		catch (...) {
			return true;
		}
	}

	// ==================== PRIVATE ====================

	std::string ShaderLoader::ReadFile(const std::string& filepath) {
		std::ifstream file(filepath);
		if (!file.is_open()) return "";

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	std::string ShaderLoader::ResolveIncludes(const std::string& source,
		const std::string& basePath) {
		std::stringstream input(source);
		std::stringstream output;
		std::string line;

		while (std::getline(input, line)) {
			if (line.find("#include") != std::string::npos) {
				size_t start = line.find('"');
				size_t end = line.rfind('"');
				if (start != std::string::npos && end != std::string::npos) {
					std::string includeFile = line.substr(start + 1, end - start - 1);

					// Исправляем путь
					std::string includePath;
					if (!basePath.empty()) {
						// Получаем директорию из basePath
						size_t lastSlash = basePath.find_last_of("/\\");
						if (lastSlash != std::string::npos) {
							std::string dir = basePath.substr(0, lastSlash + 1);
							includePath = dir + includeFile;
						}
						else {
							includePath = includeFile;
						}
					}
					else {
						includePath = includeFile;
					}

					std::string includeContent = LoadFromFile(includePath);
					if (!includeContent.empty()) {
						output << includeContent << "\n";
					}
				}
			}
			else {
				output << line << "\n";
			}
		}

		return output.str();
	}

} // namespace ogle