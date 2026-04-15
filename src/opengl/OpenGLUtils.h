#pragma once

#include "GLFunctions.h"
#include <string>
#include "../Logger.h" // Assuming Logger is available

// Macro for checking OpenGL errors
#ifdef _DEBUG
#define GL_CHECK(x) \
    x; \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            LOG_ERROR("OpenGL Error: " + std::to_string(err) + " in " + #x + " (" + __FILE__ + ":" + std::to_string(__LINE__) + ")"); \
        } \
    } while (0)
#else
#define GL_CHECK(x) x
#endif