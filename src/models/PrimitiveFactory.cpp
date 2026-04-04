#include "models/PrimitiveFactory.h"

#include "models/ModelEntity.h"

#include <cmath>
#include <vector>

namespace {
    std::shared_ptr<OGLE::ModelEntity> MakeStaticModel(
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices,
        const std::string& diffuseTexturePath)
    {
        auto model = std::make_shared<OGLE::ModelEntity>(OGLE::ModelType::STATIC);
        model->SetMeshData(vertices, indices);
        if (!diffuseTexturePath.empty()) {
            model->SetDiffuseTexturePath(diffuseTexturePath);
        }
        return model;
    }

    std::shared_ptr<OGLE::ModelEntity> CreateCube(const std::string& diffuseTexturePath)
    {
        static const std::vector<float> vertices = {
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,

            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,

            -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

             0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f
        };

        static const std::vector<unsigned int> indices = {
             0,  1,  2,   2,  3,  0,
             4,  6,  5,   6,  4,  7,
             8,  9, 10,  10, 11,  8,
            12, 14, 13,  14, 12, 15,
            16, 17, 18,  18, 19, 16,
            20, 22, 21,  22, 20, 23
        };

        return MakeStaticModel(vertices, indices, diffuseTexturePath);
    }

    std::shared_ptr<OGLE::ModelEntity> CreatePlane(const std::string& diffuseTexturePath)
    {
        static const std::vector<float> vertices = {
            -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
             0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
             0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
            -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f
        };

        static const std::vector<unsigned int> indices = {
            0, 1, 2,
            2, 3, 0
        };

        return MakeStaticModel(vertices, indices, diffuseTexturePath);
    }

    std::shared_ptr<OGLE::ModelEntity> CreateSphere(const std::string& diffuseTexturePath)
    {
        constexpr unsigned int rings = 16;
        constexpr unsigned int sectors = 32;
        constexpr float radius = 0.5f;
        constexpr float pi = 3.14159265358979323846f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve((rings + 1) * (sectors + 1) * 8);

        for (unsigned int r = 0; r <= rings; ++r) {
            const float v = static_cast<float>(r) / static_cast<float>(rings);
            const float phi = v * pi;
            const float y = std::cos(phi);
            const float sinPhi = std::sin(phi);

            for (unsigned int s = 0; s <= sectors; ++s) {
                const float u = static_cast<float>(s) / static_cast<float>(sectors);
                const float theta = u * pi * 2.0f;

                const float x = sinPhi * std::cos(theta);
                const float z = sinPhi * std::sin(theta);

                vertices.push_back(x * radius);
                vertices.push_back(y * radius);
                vertices.push_back(z * radius);

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                vertices.push_back(u);
                vertices.push_back(1.0f - v);
            }
        }

        indices.reserve(rings * sectors * 6);
        const unsigned int stride = sectors + 1;
        for (unsigned int r = 0; r < rings; ++r) {
            for (unsigned int s = 0; s < sectors; ++s) {
                const unsigned int i0 = r * stride + s;
                const unsigned int i1 = (r + 1) * stride + s;
                const unsigned int i2 = (r + 1) * stride + (s + 1);
                const unsigned int i3 = r * stride + (s + 1);

                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i2);
                indices.push_back(i3);
                indices.push_back(i0);
            }
        }

        return MakeStaticModel(vertices, indices, diffuseTexturePath);
    }
}

std::shared_ptr<OGLE::ModelEntity> PrimitiveFactory::CreatePrimitiveModel(
    OGLE::PrimitiveType type,
    const std::string& diffuseTexturePath)
{
    switch (type) {
    case OGLE::PrimitiveType::Cube:
        return CreateCube(diffuseTexturePath);
    case OGLE::PrimitiveType::Sphere:
        return CreateSphere(diffuseTexturePath);
    case OGLE::PrimitiveType::Plane:
        return CreatePlane(diffuseTexturePath);
    default:
        return nullptr;
    }
}
