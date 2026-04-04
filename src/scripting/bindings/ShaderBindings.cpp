#include "scripting/bindings/ShaderBindings.h"
#include "opengl/ShaderManager.h"
#include "Logger.h"
#include <stdexcept>

namespace OGLE
{
    namespace ScriptBindings
    {
        void RegisterShaderBindings(duktape::api_object& ns, IWorldAccess& world, PhysicsManager& physics)
        {
            ns.add_function("loadSource", [](const std::string& path) {
                try {
                    return ShaderManager::LoadShaderSource(path);
                } catch (const std::exception& e) {
                    LOG_ERROR(std::string("[JS Shader] Failed to load shader source: ") + e.what());
                    throw; // re-throw to JS
                }
            });

            ns.add_function("loadVertex", [](const std::string& name, const std::string& source) {
                return ShaderManager::GetGlobalInstance()->loadVertexShader(name, source.c_str());
            });

            ns.add_function("loadFragment", [](const std::string& name, const std::string& source) {
                return ShaderManager::GetGlobalInstance()->loadFragmentShader(name, source.c_str());
            });

            ns.add_function("link", [](const std::string& progName, const std::string& vsName, const std::string& fsName) {
                return ShaderManager::GetGlobalInstance()->linkProgram(progName, vsName, fsName);
            });

            ns.add_function("use", [](const std::string& progName) {
                return ShaderManager::GetGlobalInstance()->useProgram(progName);
            });

            ns.add_function("getProgramNames", []() {
                return ShaderManager::GetGlobalInstance()->GetProgramNames();
            });
        }
    }
}