#pragma once

#include <string>
#include <functional>
#include <memory>

struct duk_hthread;

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

        duk_hthread* GetContext() { return m_context; }

        // Get detailed error information from the last failed execution
        std::string GetLastErrorDetails() const { return m_lastError; }

    private:
        duk_hthread* m_context;
        std::string m_lastError;

        void CaptureErrorDetails();
    };
}