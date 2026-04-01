#pragma once
#ifndef SHADERMANAGER_H_
#define SHADERMANAGER_H_

#include "GLFunctions.h"
#include "../Logger.h"
#include <string>
#include <unordered_map>
#include <fstream>

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
    std::string LoadShaderSource(const std::string& filename);
private:
    std::unordered_map<std::string, GLuint> shaders;
    std::unordered_map<std::string, GLuint> programs;

    bool compileShader(const char* source, GLenum type, GLuint& shader);
    bool checkShaderCompilation(GLuint shader, const std::string& name);
    bool checkProgramLinking(GLuint program);
};

#endif // SHADERMANAGER_H_