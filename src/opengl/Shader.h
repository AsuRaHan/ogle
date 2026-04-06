#pragma once

#include "GLFunctions.h"
#include <string>
#include <map>
#include <glm/glm.hpp>

namespace OGLE {
    class Shader {
    public:
        Shader();
        ~Shader();

        // Forbid copying and moving
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) = delete;
        Shader& operator=(Shader&&) = delete;

        bool LoadFromStrings(const char* vertexShaderSource, const char* fragmentShaderSource);
        
        void Bind() const;
        void Unbind() const;

        void SetUniform(const std::string& name, int value);
        void SetUniform(const std::string& name, float value);
        void SetUniform(const std::string& name, const glm::vec2& value);
        void SetUniform(const std::string& name, const glm::vec3& value);
        void SetUniform(const std::string& name, const glm::vec4& value);
        void SetUniform(const std::string& name, const glm::mat3& value);
        void SetUniform(const std::string& name, const glm::mat4& value);
        void SetUniform(const std::string& name, const int* value, unsigned int count);
        void SetUniform(const std::string& name, const float* value, unsigned int count);


        GLuint GetProgramID() const { return m_programId; }

    private:
        bool CompileShader(const char* source, GLenum type, GLuint& shader);
        bool CheckShaderCompilation(GLuint shader, const std::string& name);
        bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);
        bool CheckProgramLinking(GLuint program);
        GLint GetUniformLocation(const std::string& name);

        GLuint m_programId;
        std::map<std::string, GLint> m_uniformLocationCache;
    };
}