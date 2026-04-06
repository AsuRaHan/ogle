#include "ModelSerializer.h"
#include "BaseModel.h"
#include "../Logger.h"
#include "../core/FileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace OGLE {
    namespace
    {
        std::string ResolveDiffuseTexturePath(
            const aiScene* scene,
            const aiMesh* mesh,
            const std::filesystem::path& modelPath)
        {
            if (!scene || !mesh || mesh->mMaterialIndex >= scene->mNumMaterials) {
                return {};
            }

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (!material || material->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
                return {};
            }

            aiString texturePath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != aiReturn_SUCCESS) {
                return {};
            }

            const std::string rawTexturePath = texturePath.C_Str();
            if (rawTexturePath.empty()) {
                return {};
            }

            if (rawTexturePath[0] == '*') {
                LOG_WARN("Embedded diffuse textures are not supported yet for: " + modelPath.string());
                return {};
            }

            const std::filesystem::path candidatePath = modelPath.parent_path() / rawTexturePath;
            return FileSystem::ResolvePath(candidatePath).string();
        }
    }

    bool ModelSerializer::LoadFromFile(BaseModel& model, const std::string& path) {
        // const std::filesystem::path resolvedPath = FileSystem::ResolvePath(path);
        // auto ext = resolvedPath.extension().string();
        // std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        // if (ext == ".omdl") {
        //     return ModelSerializer::LoadCustomFile(model, resolvedPath.string());
        // }

        // Assimp::Importer importer;
        // const aiScene* scene = importer.ReadFile(
        //     resolvedPath.string(),
        //     aiProcess_Triangulate |
        //     aiProcess_GenSmoothNormals |
        //     aiProcess_JoinIdenticalVertices |
        //     aiProcess_ImproveCacheLocality |
        //     aiProcess_FlipUVs |
        //     aiProcess_SortByPType);

        // if (!scene || !scene->HasMeshes()) {
        //     LOG_ERROR("Error loading mesh from file: " + path);
        //     return false;
        // }

        // std::vector<float> vertices;
        // std::vector<unsigned int> indices;
        // std::string diffuseTexturePath;
        // int boneCount = 0;

        // for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        //     const aiMesh* mesh = scene->mMeshes[meshIndex];
        //     if (!mesh) {
        //         continue;
        //     }

        //     boneCount += static_cast<int>(mesh->mNumBones);

        //     if (diffuseTexturePath.empty()) {
        //         diffuseTexturePath = ResolveDiffuseTexturePath(scene, mesh, resolvedPath);
        //     }

        //     const unsigned int vertexOffset = static_cast<unsigned int>(vertices.size() / 8);
        //     for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
        //         const aiVector3D& position = mesh->mVertices[vertexIndex];
        //         const aiVector3D normal = mesh->HasNormals()
        //             ? mesh->mNormals[vertexIndex]
        //             : aiVector3D(0.0f, 1.0f, 0.0f);
        //         const aiVector3D texCoord = mesh->HasTextureCoords(0)
        //             ? mesh->mTextureCoords[0][vertexIndex]
        //             : aiVector3D(0.0f, 0.0f, 0.0f);

        //         vertices.push_back(position.x);
        //         vertices.push_back(position.y);
        //         vertices.push_back(position.z);
        //         vertices.push_back(normal.x);
        //         vertices.push_back(normal.y);
        //         vertices.push_back(normal.z);
        //         vertices.push_back(texCoord.x);
        //         vertices.push_back(texCoord.y);
        //     }

        //     for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
        //         const aiFace& face = mesh->mFaces[faceIndex];
        //         if (face.mNumIndices != 3) {
        //             continue;
        //         }

        //         indices.push_back(vertexOffset + face.mIndices[0]);
        //         indices.push_back(vertexOffset + face.mIndices[1]);
        //         indices.push_back(vertexOffset + face.mIndices[2]);
        //     }
        // }

        // if (vertices.empty() || indices.empty()) {
        //     LOG_ERROR("Mesh file contains no renderable geometry: " + resolvedPath.string());
        //     return false;
        // }

        // model.SetMeshGeometry(std::move(vertices), std::move(indices));
        // model.m_boneCount = boneCount;

        // model.m_animationClips.clear();
        // if (scene->HasAnimations()) {
        //     for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
        //         const aiAnimation* aiAnim = scene->mAnimations[animIndex];
        //         if (!aiAnim) continue;

        //         AnimationClip clip;
        //         clip.name = aiAnim->mName.C_Str();
        //         if (clip.name.empty()) {
        //             clip.name = "clip_" + std::to_string(animIndex);
        //         }

        //         clip.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond != 0.0 ? aiAnim->mTicksPerSecond : 24.0);
        //         clip.duration = static_cast<float>(aiAnim->mDuration / clip.ticksPerSecond);
        //         if (clip.duration <= 0.0f) {
        //             clip.duration = 1.0f;
        //         }

        //         for (unsigned int channelIndex = 0; channelIndex < aiAnim->mNumChannels; ++channelIndex) {
        //             const aiNodeAnim* nodeAnim = aiAnim->mChannels[channelIndex];
        //             if (!nodeAnim) continue;

        //             std::map<float, AnimationKeyframe> keyframesByTime;

        //             for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
        //                 float t = static_cast<float>(nodeAnim->mPositionKeys[i].mTime / clip.ticksPerSecond);
        //                 auto& frame = keyframesByTime[t];
        //                 frame.time = t;
        //                 frame.translation = glm::vec3(nodeAnim->mPositionKeys[i].mValue.x, nodeAnim->mPositionKeys[i].mValue.y, nodeAnim->mPositionKeys[i].mValue.z);
        //             }

        //             for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
        //                 float t = static_cast<float>(nodeAnim->mRotationKeys[i].mTime / clip.ticksPerSecond);
        //                 auto& frame = keyframesByTime[t];
        //                 frame.time = t;
        //                 frame.rotation = glm::quat(nodeAnim->mRotationKeys[i].mValue.w, nodeAnim->mRotationKeys[i].mValue.x, nodeAnim->mRotationKeys[i].mValue.y, nodeAnim->mRotationKeys[i].mValue.z);
        //             }

        //             for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
        //                 float t = static_cast<float>(nodeAnim->mScalingKeys[i].mTime / clip.ticksPerSecond);
        //                 auto& frame = keyframesByTime[t];
        //                 frame.time = t;
        //                 frame.scale = glm::vec3(nodeAnim->mScalingKeys[i].mValue.x, nodeAnim->mScalingKeys[i].mValue.y, nodeAnim->mScalingKeys[i].mValue.z);
        //             }

        //             AnimationTrack track;
        //             track.nodeName = nodeAnim->mNodeName.C_Str();
        //             track.keyframes.reserve(keyframesByTime.size());
        //             for (auto& kv : keyframesByTime) {
        //                 track.keyframes.push_back(kv.second);
        //             }
        //             clip.tracks.push_back(std::move(track));
        //         }

        //         model.m_animationClips.push_back(std::move(clip));
        //     }
        // }

        // return true;
        return false;
    }

    bool ModelSerializer::SaveToCustomFile(const BaseModel& model, const std::string& path) {
        // nlohmann::json j;
        // j["version"] = 1;
        // j["mesh"] = {
        //     {"vertices", model.m_vertices},
        //     {"indices", model.m_indices}
        // };

        // j["skeleton"] = {
        //     {"boneCount", model.m_boneCount}
        // };

        // if (!model.m_animationClips.empty()) {
        //     j["animationClips"] = nlohmann::json::array();
        //     for (const auto& clip : model.m_animationClips) {
        //         nlohmann::json clipJson;
        //         clipJson["name"] = clip.name;
        //         clipJson["duration"] = clip.duration;
        //         clipJson["ticksPerSecond"] = clip.ticksPerSecond;
        //         clipJson["tracks"] = nlohmann::json::array();
        //         for (const auto& track : clip.tracks) {
        //             nlohmann::json trackJson;
        //             trackJson["nodeName"] = track.nodeName;
        //             trackJson["keyframes"] = nlohmann::json::array();
        //             for (const auto& key : track.keyframes) {
        //                 trackJson["keyframes"].push_back({
        //                     {"time", key.time},
        //                     {"translation", {key.translation.x, key.translation.y, key.translation.z}},
        //                     {"rotation", {key.rotation.w, key.rotation.x, key.rotation.y, key.rotation.z}},
        //                     {"scale", {key.scale.x, key.scale.y, key.scale.z}}
        //                 });
        //             }
        //             clipJson["tracks"].push_back(trackJson);
        //         }
        //         j["animationClips"].push_back(clipJson);
        //     }
        // }

        // std::filesystem::path filePath(path);
        // if (!FileSystem::EnsureParentDirectory(filePath)) {
        //     return false;
        // }

        // return FileSystem::WriteTextFile(filePath, j.dump(4));
        return false;
    }

    bool ModelSerializer::LoadCustomFile(BaseModel& model, const std::string& path) {
    //     std::filesystem::path filePath(path);
    //     if (!FileSystem::Exists(filePath)) {
    //         return false;
    //     }

    //     std::string content;
    //     if (!FileSystem::ReadTextFile(filePath, content)) {
    //         return false;
    //     }

    //     nlohmann::json j;
    //     try {
    //         j = nlohmann::json::parse(content);
    //     } catch (const std::exception& e) {
    //         LOG_ERROR("Failed to parse custom model file: " + std::string(e.what()));
    //         return false;
    //     }

    //     if (!j.contains("mesh") || !j["mesh"].contains("vertices") || !j["mesh"].contains("indices")) {
    //         return false;
    //     }

    //     model.m_vertices = j["mesh"]["vertices"].get<std::vector<float>>();
    //     model.m_indices = j["mesh"]["indices"].get<std::vector<unsigned int>>();

    //     model.m_boneCount = 0;
    //     if (j.contains("skeleton") && j["skeleton"].contains("boneCount")) {
    //         model.m_boneCount = j["skeleton"]["boneCount"].get<int>();
    //     }

    //     model.m_animationClips.clear();
    //     if (j.contains("animationClips") && j["animationClips"].is_array()) {
    //         for (const auto& clipJson : j["animationClips"]) {
    //             AnimationClip clip;
    //             clip.name = clipJson.value("name", std::string(""));
    //             clip.duration = clipJson.value("duration", 0.0f);
    //             clip.ticksPerSecond = clipJson.value("ticksPerSecond", 24.0f);

    //             if (clipJson.contains("tracks") && clipJson["tracks"].is_array()) {
    //                 for (const auto& trackJson : clipJson["tracks"]) {
    //                     AnimationTrack track;
    //                     track.nodeName = trackJson.value("nodeName", std::string(""));

    //                     if (trackJson.contains("keyframes") && trackJson["keyframes"].is_array()) {
    //                         for (const auto& keyJson : trackJson["keyframes"]) {
    //                             AnimationKeyframe key;
    //                             key.time = keyJson.value("time", 0.0f);
    //                             if (keyJson.contains("translation") && keyJson["translation"].is_array() && keyJson["translation"].size() == 3) {
    //                                 key.translation = glm::vec3(
    //                                     keyJson["translation"][0].get<float>(),
    //                                     keyJson["translation"][1].get<float>(),
    //                                     keyJson["translation"][2].get<float>());
    //                             }
    //                             if (keyJson.contains("rotation") && keyJson["rotation"].is_array() && keyJson["rotation"].size() == 4) {
    //                                 key.rotation = glm::quat(
    //                                     keyJson["rotation"][0].get<float>(),
    //                                     keyJson["rotation"][1].get<float>(),
    //                                     keyJson["rotation"][2].get<float>(),
    //                                     keyJson["rotation"][3].get<float>());
    //                             }
    //                             if (keyJson.contains("scale") && keyJson["scale"].is_array() && keyJson["scale"].size() == 3) {
    //                                 key.scale = glm::vec3(
    //                                     keyJson["scale"][0].get<float>(),
    //                                     keyJson["scale"][1].get<float>(),
    //                                     keyJson["scale"][2].get<float>());
    //                             }
    //                             track.keyframes.push_back(std::move(key));
    //                         }
    //                     }

    //                     clip.tracks.push_back(std::move(track));
    //                 }
    //             }

    //             model.m_animationClips.push_back(std::move(clip));
    //         }
    //     }

    //     return true;
    return false;
    }
}
