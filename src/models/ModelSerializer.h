#pragma once

#include <string>

namespace OGLE {
    class BaseModel;

    class ModelSerializer {
    public:
        static bool LoadFromFile(BaseModel& model, const std::string& path);
        static bool LoadCustomFile(BaseModel& model, const std::string& path);
        static bool SaveToCustomFile(const BaseModel& model, const std::string& path);
    };
}
