#pragma once

#include <string>
#include <duktape.h>
#include <dukglue/dukglue.h>

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

        bool CallGlobalFunction(const char* functionName);
        bool CallGlobalFunction(const char* functionName, float argument);
        bool CallGlobalFunction(const char* functionName, unsigned int first, unsigned int second);

        template<typename Function>
        bool RegisterFunction(const char* name, Function function)
        {
            if (!IsContextValid()) {
                return false;
            }
            dukglue_register_function(m_context, function, name);
            return true;
        }

        template<typename ClassType, typename... Args>
        bool RegisterConstructor(const char* name)
        {
            if (!IsContextValid()) {
                return false;
            }
            dukglue_register_constructor<ClassType, Args...>(m_context, name);
            return true;
        }

        template<typename ClassType, typename Method>
        bool RegisterMethod(const char* name, Method method)
        {
            if (!IsContextValid()) {
                return false;
            }
            dukglue_register_method(m_context, method, name);
            return true;
        }

        template<typename T>
        bool PushGlobalObject(const char* name, T* object)
        {
            if (!IsContextValid()) {
                return false;
            }

            dukglue_push(m_context, object);
            duk_put_global_string(m_context, name);
            return true;
        }

        const std::string& GetLastErrorDetails() const { return m_lastError; }

    private:
        duk_context* m_context = nullptr;
        std::string m_lastError;

        bool IsContextValid() const { return m_context != nullptr; }
        void CaptureErrorDetails();
    };
}
