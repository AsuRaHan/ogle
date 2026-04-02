#include "scripting/ScriptEngine.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace OGLE
{
    ScriptEngine::ScriptEngine()
    {
        m_context = duk_create_heap_default();
        if (!m_context) {
            LOG_ERROR("Failed to create Duktape heap");
            throw std::runtime_error("Failed to create Duktape heap");
        }
    }

    ScriptEngine::~ScriptEngine()
    {
        if (m_context) {
            duk_destroy_heap(m_context);
            m_context = nullptr;
        }
    }

    bool ScriptEngine::ExecuteString(const std::string& source, const std::string& filename)
    {
        duk_push_string(m_context, filename.c_str());
        if (duk_pcompile_lstring_filename(m_context, 0, source.c_str(), source.length()) != 0) {
            LOG_ERROR("Script compilation failed: " + std::string(duk_safe_to_string(m_context, -1)));
            duk_pop(m_context);
            return false;
        }

        if (duk_pcall(m_context, 0) != 0) {
            LOG_ERROR("Script execution failed: " + std::string(duk_safe_to_string(m_context, -1)));
            duk_pop(m_context);
            return false;
        }

        duk_pop(m_context); // Pop result
        return true;
    }

    bool ScriptEngine::ExecuteFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open script file: " + filepath);
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return ExecuteString(buffer.str(), filepath);
    }

}