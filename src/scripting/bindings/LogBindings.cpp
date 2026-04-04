#include "scripting/bindings/LogBindings.h"
#include "Logger.h"

namespace OGLE { namespace ScriptBindings {

duk_ret_t JsLog(duk_context* context)
{
    const char* message = duk_safe_to_string(context, 0);
    LOG_INFO(std::string("[JS] ") + (message ? message : ""));
    return 0;
}

void RegisterLogBindings(duk_context* ctx, const NativeBinder& bindNative) {
    bindNative("log", JsLog, 1);
}

}}