#ifndef SPENNY_MODEL_H
#define SPENNY_MODEL_H

#include <vector>
#include <optional>

#include "spennymath.h"
#include "spennytypes.h"
#include "texture.h"
#include "vertbuf.h"

namespace sr
{

struct Vertex
{
    using Layout = BufferLayout<F32Component<3>,
                                F32Component<3>,
                                F32Component<3>,
                                F32Component<3>,
                                F32Component<2>>;

    sm::Vec3 pos;
    sm::Vec3 norm;
    sm::Vec3 tan;
    sm::Vec3 bitan;
    sm::Vec2 uv;
};

typedef isize MaterialIndex;

struct Mesh
{
    MaterialIndex material_index;
    std::vector<Vertex> verts;
    std::vector<u32> indices;
};

struct Material
{
    f32 metallic;
    f32 roughness;
    Texture diffuse;
    Texture normals;
    // stats...
};

struct Model
{
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    // materials
};

class ModelLoader
{
public:
    std::optional<Model> load_from_file(const std::string& filename);
};


struct meshdata
{
    u32 vao;
    u32 elements;
    u32 mat_idx;
};

} // namespace sr

#endif // SPENNY_MODEL_H
