#include "BaseModel.h"
#include "../Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <OpenMesh/Core/IO/MeshIO.hh>

namespace OGLE {
    BaseModel::BaseModel() : m_Mesh(std::make_unique<MyMesh>()) {}

    BaseModel::~BaseModel() {}

    bool BaseModel::LoadFromFile(const std::string& path) {
        if (!OpenMesh::IO::read_mesh(*m_Mesh, path)) {
            LOG_ERROR("Error loading mesh from file: " + path);
            return false;
        }
        return true;
    }

    void BaseModel::BakeToGPU() {
        if (!m_Mesh) {
            LOG_ERROR("Cannot bake to GPU, mesh data is missing.");
            return;
        }

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        m_Mesh->request_vertex_normals();
        m_Mesh->request_vertex_texcoords2D();
        m_Mesh->update_normals();

        for (MyMesh::VertexIter v_it = m_Mesh->vertices_begin(); v_it != m_Mesh->vertices_end(); ++v_it) {
            vertices.push_back(m_Mesh->point(*v_it)[0]);
            vertices.push_back(m_Mesh->point(*v_it)[1]);
            vertices.push_back(m_Mesh->point(*v_it)[2]);

            vertices.push_back(m_Mesh->normal(*v_it)[0]);
            vertices.push_back(m_Mesh->normal(*v_it)[1]);
            vertices.push_back(m_Mesh->normal(*v_it)[2]);

            if (m_Mesh->has_vertex_texcoords2D()) {
                vertices.push_back(m_Mesh->texcoord2D(*v_it)[0]);
                vertices.push_back(m_Mesh->texcoord2D(*v_it)[1]);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }

        for (MyMesh::FaceIter f_it = m_Mesh->faces_begin(); f_it != m_Mesh->faces_end(); ++f_it) {
            for (MyMesh::FaceVertexIter fv_it = m_Mesh->fv_iter(*f_it); fv_it.is_valid(); ++fv_it) {
                indices.push_back(fv_it->idx());
            }
        }

        m_MeshBuffer = std::make_unique<MeshBuffer>();
        m_MeshBuffer->Create(vertices, indices);
    }
}
