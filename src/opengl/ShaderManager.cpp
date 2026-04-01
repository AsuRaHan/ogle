#include "ShaderManager.h"


ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {
    for (auto& pair : shaders) {
        if (pair.second != 0) { // Проверяем, что шейдер существует
            GL_CHECK(glDeleteShader(pair.second));
        }
    }
    for (auto& pair : programs) {
        if (pair.second != 0) { // Проверяем, что программа существует
            GL_CHECK(glDeleteProgram(pair.second));
        }
    }
}

bool ShaderManager::loadVertexShader(const std::string& name, const char* source) {
    LOG_INFO("Начинаю загрузку вершинного шейдера " + name);
    GLuint shader;
    if (!compileShader(source, GL_VERTEX_SHADER, shader)) {
        return false;
    }
    shaders[name] = shader;
    return true;
}

bool ShaderManager::loadFragmentShader(const std::string& name, const char* source) {
    LOG_INFO("Начинаю загрузку фрагментного шейдера " + name);
    GLuint shader;
    if (!compileShader(source, GL_FRAGMENT_SHADER, shader)) {
        return false;
    }
    shaders[name] = shader;
    return true;
}

bool ShaderManager::loadGeometryShader(const std::string& name, const char* source) {
    LOG_INFO("Начинаю загрузку геометрического шейдера " + name);
    GLuint shader;
    if (!compileShader(source, GL_GEOMETRY_SHADER, shader)) {
        return false;
    }
    shaders[name] = shader;
    return true;
}

bool ShaderManager::loadTessControlShader(const std::string& name, const char* source) {
    LOG_INFO("Начинаю загрузку контрольного шейдера тесселяции " + name);
    GLuint shader;
    if (!compileShader(source, GL_TESS_CONTROL_SHADER, shader)) {
        return false;
    }
    shaders[name] = shader;
    return true;
}

bool ShaderManager::loadTessEvaluationShader(const std::string& name, const char* source) {
    LOG_INFO("Начинаю загрузку оценочного шейдера тесселяции " + name);
    GLuint shader;
    if (!compileShader(source, GL_TESS_EVALUATION_SHADER, shader)) {
        return false;
    }
    shaders[name] = shader;
    return true;
}

bool ShaderManager::loadComputeShader(const std::string& name, const char* source) {
    LOG_INFO("Начинаю загрузку вычислительного шейдера " + name);
    // Удаляем старый шейдер, если он есть
    if (shaders.find(name) != shaders.end()) {
        GL_CHECK(glDeleteShader(shaders[name]));
        shaders.erase(name);
    }

    GLuint shader;
    if (!compileShader(source, GL_COMPUTE_SHADER, shader)) {
        return false;
    }
    shaders[name] = shader;
    return true;
}



bool ShaderManager::linkProgram(const std::string& programName, const std::string& vertexShaderName, const std::string& fragmentShaderName) {
    LOG_INFO(std::string("[ShaderManager::linkProgram] Начинаю компиляцию шейдерной программы programName=") + programName 
        + " vertexShaderName=" + vertexShaderName 
        + " fragmentShaderName=" + fragmentShaderName);
    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    if (!checkProgramLinking(program)) {
        GL_CHECK(glDeleteProgram(program));
        return false;
    }
    programs[programName] = program;
    LOG_INFO(std::string("[ShaderManager::linkProgram] Компиляция шейдерной программы programName=") + programName + " завершена ей присвоен ID=" + std::to_string(program));
    return true;
}

bool ShaderManager::linkGeometryProgram(const std::string& programName, const std::string& vertexShaderName, 
    const std::string& geometryShaderName, const std::string& fragmentShaderName) {

    LOG_INFO(std::string("[ShaderManager::linkGeometryProgram] Начинаю компиляцию геометрической шейдерной программы programName=") + programName
        + " vertexShaderName=" + vertexShaderName
        + " geometryShaderName=" + geometryShaderName
        + " fragmentShaderName=" + fragmentShaderName);
    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[geometryShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    if (!checkProgramLinking(program)) {
        GL_CHECK(glDeleteProgram(program));
        return false;
    }
    programs[programName] = program;
    LOG_INFO(std::string("[ShaderManager::linkGeometryProgram] Компиляция шейдерной программы programName=") + programName + " завершена ей присвоен ID=" + std::to_string(program));
    return true;
}

bool ShaderManager::linkTessellationProgram(const std::string& programName, const std::string& vertexShaderName, 
    const std::string& tessControlShaderName, const std::string& tessEvaluationShaderName, const std::string& fragmentShaderName) {

    LOG_INFO(std::string("[ShaderManager::linkTessellationProgram] Начинаю компиляцию шейдерной программы тесселяции programName=") + programName
        + " vertexShaderName=" + vertexShaderName
        + " tessControlShaderName=" + tessControlShaderName
        + " tessEvaluationShaderName=" + tessEvaluationShaderName
        + " fragmentShaderName=" + fragmentShaderName);

    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[vertexShaderName]);
    glAttachShader(program, shaders[tessControlShaderName]);
    glAttachShader(program, shaders[tessEvaluationShaderName]);
    glAttachShader(program, shaders[fragmentShaderName]);
    glLinkProgram(program);
    if (!checkProgramLinking(program)) {
        GL_CHECK(glDeleteProgram(program));
        return false;
    }
    programs[programName] = program;
    LOG_INFO(std::string("[ShaderManager::linkTessellationProgram] Компиляция шейдерной программы programName=") + programName + " завершена ей присвоен ID=" + std::to_string(program));
    return true;
}


bool ShaderManager::linkComputeProgram(const std::string& programName, const std::string& computeShaderName) {
    LOG_INFO(std::string("[ShaderManager::linkComputeProgram] Начинаю компиляцию шейдерной программы programName=") + programName
        + " computeShaderName=" + computeShaderName);
    // Удаляем старую программу, если она есть
    if (programs.find(programName) != programs.end()) {
        GL_CHECK(glDeleteProgram(programs[programName]));
        programs.erase(programName);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, shaders[computeShaderName]);
    glLinkProgram(program);
    if (!checkProgramLinking(program)) {
        GL_CHECK(glDeleteProgram(program));
        return false;
    }
    programs[programName] = program;
    LOG_INFO(std::string("[ShaderManager::linkComputeProgram] Компиляция шейдерной программы programName=") + programName + " завершена ей присвоен ID=" + std::to_string(program));
    return true;
}

bool ShaderManager::useProgram(const std::string& programName) {
    auto it = programs.find(programName);
    if (it == programs.end()) {
        LOG_ERROR("ShaderManager::useProgram: program not found: " + programName);
        return false;
    }
    glUseProgram(it->second);
    return true;
}

GLuint ShaderManager::getProgram(const std::string& programName) const {
    return programs.at(programName);
}

bool ShaderManager::compileShader(const char* source, GLenum type, GLuint& shader) {
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return checkShaderCompilation(shader, "Shader");
}

bool ShaderManager::checkShaderCompilation(GLuint shader, const std::string& name) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        LOG_ERROR(std::string("[ShaderManager::checkShaderCompilation] Ошибка компиляции ") + name + ": " + infoLog);
        return false;
    }
    return true;
}

bool ShaderManager::checkProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        LOG_ERROR(std::string("[ShaderManager::checkProgramLinking] Ошибка линковки программы: ") + infoLog + " ID=" + std::to_string(program));
        return false;
    }
    return true;
}
std::string ShaderManager::LoadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось загрузить файл шейдерной программы: " + filename);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}