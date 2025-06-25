#ifndef SPENNY_TEXTURE_H
#define SPENNY_TEXTURE_H

#include <vector>
#include <string>
#include <glad/glad.h>
#include "spennytypes.h"
#include "shader.h"

namespace sr
{

class Texture
{
public:
    void alloc_texture(i32 w,
                       i32 h,
                       u32 wrap = GL_CLAMP_TO_EDGE,
                       u32 filter = GL_LINEAR);

    void load_texture(i32 w,
                      i32 h,
                      u8* data,
                      u32 src_format = GL_SRGB_ALPHA,
                      u32 wrap = GL_CLAMP_TO_EDGE,
                      u32 filter = GL_LINEAR);

    void bind_texture(u32 slot);

    void unbind();

    GLuint get_id() const noexcept;

private:
    friend class TextureBuilder;
    i32 w;
    i32 h;
    GLuint id;
};

class TextureBuilder
{
public:
    TextureBuilder()
        : level(0),
          internal_format(GL_RGBA),
          width(0),
          height(0),
          src_format(GL_SRGB_ALPHA),
          filter(GL_LINEAR),
          wrap(GL_CLAMP_TO_EDGE),
          data_type(GL_UNSIGNED_BYTE),
          data(nullptr)
        {
        }

    TextureBuilder& with_level(u32 l) { level = l; return *this; }
    TextureBuilder& with_internal_format(u32 format) { internal_format = format; return *this; }
    TextureBuilder& with_width(u32 w) { width = w; return *this; }
    TextureBuilder& with_height(u32 h) { height = h; return *this; }
    TextureBuilder& with_src_format(u32 s) { src_format = s; return *this; }
    TextureBuilder& with_filter(u32 f) { filter = f; return *this; }
    TextureBuilder& with_wrap(u32 w) { wrap = w; return *this; }
    TextureBuilder& with_data_type(u32 d) { data_type = d; return *this; }
    TextureBuilder& with_data(void* d) { data = d; return *this; }

    Texture build();
    void build_into(Texture& tex);

private:
    u32 level;
    u32 internal_format;
    u32 width;
    u32 height;
    u32 src_format;
    u32 filter;
    u32 wrap;
    u32 data_type;
    void* data;
};

enum CubemapFace
{
    CubemapFace_Right = 0,
    CubemapFace_Left,
    CubemapFace_Top,
    CubemapFace_Bottom,
    CubemapFace_Front,
    CubemapFace_Back,
    CubemapFace_NFaces
};

class Cubemap
{
public:

    Cubemap()
        : buffered_faces(0), id(0)
    {
        glGenTextures(1, &id);
        assert(id && "id was bad?");

        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void buffer_face(i32 face, i32 w, i32 h, const u8* data);

    void bind();
    void unbind();

    inline GLuint get_id()
    {
        return id;
    }

    inline bool is_buffered()
    {
        return buffered_faces == (1 << CubemapFace_NFaces) - 1;
    }

private:
    u32 buffered_faces;
    GLuint id;
};

class Skybox
{
public:

    Skybox() : shader(), cubemap() {}

    void load_from_images(const std::vector<std::string>& images);
    void load_from_dir(const std::string& images);
    void load_from_hdr(const std::string& hdr);

    void render(sm::Mat4 view_mat, sm::Mat4 proj_mat);

private:

    static GLuint cube_vao;

    void buffer_cube();

    Shader shader;
    Cubemap cubemap;
};

} // namespace sr
#endif // SPENNY_TEXTURE_H
