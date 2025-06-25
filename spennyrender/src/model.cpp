#include "model.h"

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include "spennytypes.h"
#include "texture.h"

namespace sr
{

void load_meshes_from_node(const aiNode* node, const aiScene* ai_scene, Model* model)
{
    for (u32 mesh_idx = 0; mesh_idx < node->mNumMeshes; mesh_idx++)
    {
        Mesh mesh;
        auto idx = node->mMeshes[mesh_idx];
        auto ai_mesh = ai_scene->mMeshes[idx];
        mesh.verts.resize(ai_mesh->mNumVertices);

        for (u32 vi = 0; vi < ai_mesh->mNumVertices; vi++)
        {
            auto& vert = mesh.verts[vi];
            vert.pos.x = ai_mesh->mVertices[vi].x;
            vert.pos.y = ai_mesh->mVertices[vi].y;
            vert.pos.z = ai_mesh->mVertices[vi].z;

            vert.norm.x = ai_mesh->mNormals[vi].x;
            vert.norm.y = ai_mesh->mNormals[vi].y;
            vert.norm.z = ai_mesh->mNormals[vi].z;

            vert.tan.x = ai_mesh->mTangents[vi].x;
            vert.tan.y = ai_mesh->mTangents[vi].y;
            vert.tan.z = ai_mesh->mTangents[vi].z;

            vert.bitan.x = ai_mesh->mBitangents[vi].x;
            vert.bitan.y = ai_mesh->mBitangents[vi].y;
            vert.bitan.z = ai_mesh->mBitangents[vi].z;

            if (ai_mesh->mTextureCoords[0])
            {
                vert.uv.u = ai_mesh->mTextureCoords[0][vi].x;
                vert.uv.v = ai_mesh->mTextureCoords[0][vi].y;
            }
        }

        for (u32 face = 0; face < ai_mesh->mNumFaces; face++)
        {
            auto ai_face = ai_mesh->mFaces[face];
            for (u32 index = 0; index < ai_face.mNumIndices; index++)
            {
                mesh.indices.push_back(ai_face.mIndices[index]);
            }
        }
        mesh.material_index = ai_mesh->mMaterialIndex;
        model->meshes.push_back(mesh);
    }
}

Texture load_embedded_texture(const aiScene* scene, const aiString* tex_name, bool is_linear = false);

Texture load_embedded_texture(const aiScene* scene, const aiString* tex_name, bool is_linear)
{
    auto name_cstr = tex_name->C_Str();

    auto idx = atoi(name_cstr + 1);

    auto ai_tex = scene->mTextures[idx];

    if (ai_tex->mHeight == 0)
    {
        int w, h, c;
        auto data = stbi_load_from_memory(reinterpret_cast<u8*>(ai_tex->pcData),
                                          ai_tex->mWidth,
                                          &w,
                                          &h,
                                          &c,
                                          4);
        if (data == nullptr)
        {
            std::cout << "Loading the texture failed!" << std::endl;
            std::cout << stbi_failure_reason() << std::endl;
            return Texture{};
        }
        u32 format = is_linear ? GL_RGBA : GL_SRGB_ALPHA;
        Texture result;
        result.load_texture(w, h, data, format);
        return result;
    }
    else
    {
        std::cout << "TODO external texture" << std::endl;
    }

    return Texture{};
}

void load_materials(const aiScene* scene, Model* model)
{
    model->materials.resize(scene->mNumMaterials);
    for (u32 mat_idx = 0; mat_idx < scene->mNumMaterials; mat_idx++)
    {
        auto ai_material = scene->mMaterials[mat_idx];
        auto& material = model->materials[mat_idx];
        auto metalp = &material.metallic;
        auto roughp = &material.roughness;

        ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metalp);
        ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughp);

        if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString diffuse_file;
            ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_file);

            if (diffuse_file.data[0] == '*')
            {
                material.diffuse = load_embedded_texture(scene, &diffuse_file);
            }
        }
        if (ai_material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            aiString normals_file;
            ai_material->GetTexture(aiTextureType_NORMALS, 0, &normals_file);

            if (normals_file.data[0] == '*')
            {
                material.normals = load_embedded_texture(scene, &normals_file, true);
            }
        }
    }
}

std::optional<Model> ModelLoader::load_from_file(const std::string& filename)
{
    Assimp::Importer importer;

    const auto scene = importer.ReadFile(filename,
                                         aiProcess_Triangulate |
                                         aiProcess_CalcTangentSpace |
                                         aiProcess_FlipUVs |
                                         aiProcess_FixInfacingNormals |
                                         aiProcess_OptimizeGraph);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "didn't work: " << importer.GetErrorString() << std::endl;
        return std::nullopt;
    }
    auto root_node = scene->mRootNode;
    if (!scene->HasMeshes())
    {
        std::cout << "no meshes?" << std::endl;
        return std::nullopt;
    }

    Model result;

    std::vector<aiNode*> node_stack;
    node_stack.push_back(root_node);

    while (node_stack.size() > 0)
    {
        auto current = node_stack.back();

        load_meshes_from_node(current, scene, &result);

        node_stack.pop_back();
        for (usize child = 0;
             child < current->mNumChildren;
             child++)
        {
            node_stack.push_back(current->mChildren[child]);
        }
    }

    load_materials(scene, &result);

    return result;
}

} // namespace td
