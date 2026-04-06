#include "Shader.h"
#include "GLFunctions.h"
#include "../Logger.h"
#include <glm/gtc/type_ptr.hpp>

namespace OGLE {

Shader::Shader(GLuint programId)
    : m_programId(programId)
{
    if (m_programId == 0) {
        LOG_ERROR("Shader constructed with invalid program ID");
    }
}

Shader::~Shader() {
    // Program deletion is handled by ShaderManager, so we do not delete it here.
}

void Shader::Bind() const {
    if (IsValid()) {
        glUseProgram(m_programId);
    }
}

void Shader::Unbind() const {
    glUseProgram(0);
}

GLint Shader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    GLint location = glGetUniformLocation(m_programId, name.c_str());
    if (location != -1) {
        m_uniformCache[name] = location;
    }
    return location;
}

void Shader::SetUniform(const std::string& name, int value) {
    GLint loc = GetUniformLocation(name);
    if (loc != -1) glUniform1i(loc, value);
}

void Shader::SetUniform(const std::string& name, float value) {
    GLint loc = GetUniformLocation(name);
    if (loc != -1) glUniform1f(loc, value);
}

void Shader::SetUniform(const std::string& name, const glm::vec2& value) {
    GLint loc = GetUniformLocation(name);
    if (loc != -1) glUniform2f(loc, value.x, value.y);
}

void Shader::SetUniform(const std::string& name, const glm::vec3& value) {
    GLint loc = GetUniformLocation(name);
    if (loc != -1) glUniform3f(loc, value.x, value.y, value.z);
}

void Shader::SetUniform(const std::string& name, const glm::vec4& value) {
    GLint loc = GetUniformLocation(name);
    if (loc != -1) glUniform4f(loc, value.x, value.y, value.z, value.w);
}

void Shader::SetUniform(const std::string& name, const glm::mat4& value) {
    GLint loc = GetUniformLocation(name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace OGLE
