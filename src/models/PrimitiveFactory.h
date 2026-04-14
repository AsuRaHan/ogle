#pragma once

#include <memory>
#include <string>

#include "world/WorldComponents.h"

namespace OGLE {
    class ModelEntity;
    class Material;
}

class PrimitiveFactory
{
public:
    static std::shared_ptr<OGLE::ModelEntity> CreatePrimitiveModel(
        OGLE::PrimitiveType type,
        const OGLE::Material* material = nullptr);
};
