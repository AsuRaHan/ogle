#pragma once
#include "GLFunctions.h"

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <GL/glew.h> // or GL/gl.h depending on setup

namespace OGLE {
class Shader {
private:
    GLint GetUniformLocation(const std::string& name);
    GLuint m_programId;
    std::unordered_map<std::string, GLint> m_uniformCache; // name -> location
public:
    explicit Shader(GLuint programId);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) noexcept = delete;
    Shader& operator=(Shader&&) noexcept = delete;

    void Bind() const;
    void Unbind() const;
    bool IsValid() const { return m_programId != 0; }

    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, const glm::vec2& value);
    void SetUniform(const std::string& name, const glm::vec3& value);
    void SetUniform(const std::string& name, const glm::vec4& value);
    void SetUniform(const std::string& name, const glm::mat4& value);

    GLuint GetProgramId() const { return m_programId; }

};
}
