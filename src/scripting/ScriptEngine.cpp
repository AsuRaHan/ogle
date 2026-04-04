#include "scripting/ScriptEngine.h"
#include "core/FileSystem.h"
#include "Logger.h"
#include <duktape.h>
#include <sstream>
#include <stdexcept>

namespace OGLE
{
    ScriptEngine::ScriptEngine()
        : m_context(nullptr), m_lastError("")
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

    void ScriptEngine::CaptureErrorDetails()
    {
        if (!duk_is_object(m_context, -1)) {
            m_lastError = std::string(duk_safe_to_string(m_context, -1));
            return;
        }

        std::ostringstream oss;
        
        duk_get_prop_string(m_context, -1, "message");
        const char* message = duk_to_string(m_context, -1);
        oss << "Error: " << (message ? message : "Unknown error");
        duk_pop(m_context);

        duk_get_prop_string(m_context, -1, "stack");
        const char* stack = duk_to_string(m_context, -1);
        if (stack) {
            oss << "\nStack trace:\n" << stack;
        }
        duk_pop(m_context);

        duk_get_prop_string(m_context, -1, "lineNumber");
        if (duk_is_number(m_context, -1)) {
            oss << "\nLine: " << duk_to_number(m_context, -1);
        }
        duk_pop(m_context);

        m_lastError = oss.str();
    }

    bool ScriptEngine::ExecuteString(const std::string& source, const std::string& filename)
    {
        m_lastError.clear();

        duk_push_string(m_context, filename.c_str());
        if (duk_pcompile_lstring_filename(m_context, 0, source.c_str(), source.length()) != 0) {
            CaptureErrorDetails();
            LOG_ERROR("[Script Compilation Error] " + filename + ":\n" + m_lastError);
            duk_pop(m_context);
            return false;
        }

        if (duk_pcall(m_context, 0) != 0) {
            CaptureErrorDetails();
            LOG_ERROR("[Script Runtime Error] " + filename + ":\n" + m_lastError);
            duk_pop(m_context);
            return false;
        }

        duk_pop(m_context); // Pop result
        return true;
    }

    bool ScriptEngine::ExecuteFile(const std::string& filepath)
    {
        m_lastError.clear();

        // Check if file exists first
        std::filesystem::path filePath(filepath);
        if (!FileSystem::Exists(filePath)) {
            m_lastError = "Script file not found: " + filepath;
            LOG_ERROR("Script file not found: " + filepath);
            return false;
        }

        // Read file content using FileSystem
        std::string content;
        if (!FileSystem::ReadTextFile(filePath, content)) {
            m_lastError = "Failed to read script file: " + filepath;
            LOG_ERROR("Failed to read script file: " + filepath);
            return false;
        }

        return ExecuteString(content, filepath);
    }
}