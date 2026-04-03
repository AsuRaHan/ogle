#pragma once

#include <duktape.h>
#include <string>
#include <functional>

namespace OGLE
{
    class ScriptEngine
    {
    public:
        ScriptEngine();
        ~ScriptEngine();

        ScriptEngine(const ScriptEngine&) = delete;
        ScriptEngine& operator=(const ScriptEngine&) = delete;

        bool ExecuteString(const std::string& source, const std::string& filename = "eval");
        bool ExecuteFile(const std::string& filepath);

        duk_context* GetContext() { return m_context; }

        // Get detailed error information from the last failed execution
        std::string GetLastErrorDetails() const { return m_lastError; }

        template<typename T>
        void SetGlobalPointer(const char* name, T* ptr)
        {
            duk_push_pointer(m_context, ptr);
            duk_put_global_string(m_context, name);
        }

    private:
        duk_context* m_context;
        std::string m_lastError;

        // Helper to extract detailed error information from Duktape
        void CaptureErrorDetails();
    };
}