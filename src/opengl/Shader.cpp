#include "Shader.h"
#include "../Logger.h"
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace OGLE {
    Shader::Shader() : m_programId(0) {}

    Shader::~Shader() {
        if (m_programId != 0) {
            glDeleteProgram(m_programId);
        }
    }

    bool Shader::LoadFromStrings(const char* vertexShaderSource, const char* fragmentShaderSource) {
        GLuint vertexShader, fragmentShader;
        if (!CompileShader(vertexShaderSource, GL_VERTEX_SHADER, vertexShader)) {
            return false;
        }
        if (!CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER, fragmentShader)) {
            glDeleteShader(vertexShader);
            return false;
        }

        if (!LinkProgram(vertexShader, fragmentShader)) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return false;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return true;
    }

    void Shader::Bind() const {
        glUseProgram(m_programId);
    }

    void Shader::Unbind() const {
        glUseProgram(0);
    }

    void Shader::SetUniform(const std::string& name, int value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform1i(location, value);
        }
    }

    void Shader::SetUniform(const std::string& name, float value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform1f(location, value);
        }
    }

    void Shader::SetUniform(const std::string& name, const glm::vec2& value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform2fv(location, 1, glm::value_ptr(value));
        }
    }

    void Shader::SetUniform(const std::string& name, const glm::vec3& value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform3fv(location, 1, glm::value_ptr(value));
        }
    }

    void Shader::SetUniform(const std::string& name, const glm::vec4& value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform4fv(location, 1, glm::value_ptr(value));
        }
    }

    void Shader::SetUniform(const std::string& name, const glm::mat3& value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

    void Shader::SetUniform(const std::string& name, const glm::mat4& value) {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

    void Shader::SetUniform(const std::string& name, const int* value, unsigned int count)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform1iv(location, count, value);
        }
    }

    void Shader::SetUniform(const std::string& name, const float* value, unsigned int count)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) {
            glUniform1fv(location, count, value);
        }
    }

    bool Shader::CompileShader(const char* source, GLenum type, GLuint& shader) {
        shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        return CheckShaderCompilation(shader, "Shader");
    }

    bool Shader::CheckShaderCompilation(GLuint shader, const std::string& name) {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            LOG_ERROR(std::string("Shader compilation error for ") + name + ": " + infoLog);
            return false;
        }
        return true;
    }

    bool Shader::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
        m_programId = glCreateProgram();
        glAttachShader(m_programId, vertexShader);
        glAttachShader(m_programId, fragmentShader);
        glLinkProgram(m_programId);
        return CheckProgramLinking(m_programId);
    }

    bool Shader::CheckProgramLinking(GLuint program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            LOG_ERROR(std::string("Shader program linking error: ") + infoLog);
            glDeleteProgram(m_programId);
            m_programId = 0;
            return false;
        }
        return true;
    }

    GLint Shader::GetUniformLocation(const std::string& name) {
        if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
            return m_uniformLocationCache[name];
        }

        GLint location = glGetUniformLocation(m_programId, name.c_str());
        if (location == -1) {
            // LOG_WARN("Uniform '" + name + "' not found in shader program.");
        }
        m_uniformLocationCache[name] = location;
        return location;
    }
}