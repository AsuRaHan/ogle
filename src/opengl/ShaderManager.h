#pragma once

#include "GLFunctions.h"
#include "../Logger.h"
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <streambuf>

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    bool loadVertexShader(const std::string& name, const char* source);
    bool loadFragmentShader(const std::string& name, const char* source);
    bool loadGeometryShader(const std::string& name, const char* source);
    bool loadTessControlShader(const std::string& name, const char* source);
    bool loadTessEvaluationShader(const std::string& name, const char* source);
    bool loadComputeShader(const std::string& name, const char* source);

    bool linkProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& fragmentShaderName);
    bool linkGeometryProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& geometryShaderName, const std::string& fragmentShaderName);
    bool linkTessellationProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& tessControlShaderName, const std::string& tessEvaluationShaderName, const std::string& fragmentShaderName);
    bool linkComputeProgram(const std::string& programName, const std::string& computeShaderName);

    bool useProgram(const std::string& programName);
    GLuint getProgram(const std::string& programName) const;
    bool hasProgram(const std::string& programName) const;
    std::vector<std::string> GetProgramNames() const;
    GLint getUniformLocation(const std::string& programName, const std::string& uniformName);

    static void SetGlobalInstance(ShaderManager* instance);
    static ShaderManager* GetGlobalInstance();

private:
    static ShaderManager* s_globalInstance;

public:
    static std::string LoadShaderSource(const std::string& filename);

private:
    bool compileShader(const char* source, GLenum type, GLuint& shader);
    bool checkShaderCompilation(GLuint shader, const std::string& name);
    bool checkProgramLinking(GLuint program);
    void cacheUniforms(const std::string& programName, GLuint programId);

    std::map<std::string, GLuint> shaders;
    std::map<std::string, GLuint> programs;
    std::map<std::string, std::map<std::string, GLint>> m_uniformLocations;
};