#pragma once

#include <memory>
#include <string>

#include "world/WorldComponents.h"

namespace OGLE {
    class ModelEntity;
}

class PrimitiveFactory
{
public:
    static std::shared_ptr<OGLE::ModelEntity> CreatePrimitiveModel(
        OGLE::PrimitiveType type,
        const std::string& diffuseTexturePath = "");
};

