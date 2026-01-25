#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include "render/shader/ShaderProgram.h"

namespace ogle {

class TestCube {
public:
    enum class ShaderMode {
        Colorful,      // наш кастомный разноцветный
        BasicColor,    // builtin BasicColor
        BasicTexture,  // builtin BasicTexture (без текстуры)
        Unlit,         // builtin Unlit
        Wireframe      // builtin Wireframe
    };

    void SetShaderMode(ShaderMode mode);

    TestCube();
    ~TestCube();
    
    bool Initialize();
    void Shutdown();
    void Render(float time, const glm::mat4& view, const glm::mat4& projection);
    bool TestBuiltinShaders();
    void SetupInputActions();

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    
    std::shared_ptr<ShaderProgram> m_shaderProgram;
    
    void CreateGeometry();
};

} // namespace ogle