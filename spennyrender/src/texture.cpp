#include <string>
#include <stb_image.h>
#include "renderer.h"
#include "spennymath.h"
#include "framebuf.h"
#include "texture.h"
#include "spennytypes.h"

namespace sr
{

void Texture::alloc_texture(i32 w, i32 h, u32 wrap, u32 filter)
{
    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_SRGB_ALPHA,
                 w,
                 h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    this->w = w;
    this->h = h;
}

void Texture::load_texture(i32 w, i32 h, u8* data, u32 src_fmt, u32 wrap, u32 filter)
{
    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 src_fmt,
                 w,
                 h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    this->w = w;
    this->h = h;
}

void Texture::bind_texture(u32 slot)
{
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

Texture TextureBuilder::build()
{
    Texture result;
    result.w = width;
    result.h = height;
    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexImage2D(GL_TEXTURE_2D,
                 level,
                 internal_format,
                 width,
                 height,
                 0,
                 src_format,
                 data_type,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return result;
}

void TextureBuilder::build_into(Texture& result)
{
    result.w = width;
    result.h = height;
    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexImage2D(GL_TEXTURE_2D,
                 level,
                 internal_format,
                 width,
                 height,
                 0,
                 src_format,
                 data_type,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::get_id() const noexcept
{
    return id;
}

void Cubemap::bind()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void Cubemap::unbind()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::buffer_face(i32 face, i32 w, i32 h, const u8* data)
{
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

const char* skybox_vs = R"SRC(
#version 400 core

layout (location = 0) in vec3 position;

layout (std140) uniform GlobalUniforms
{
    vec4 clip;
    vec4 camera_pos;
    vec4 material_props;
    mat4 view;
    mat4 perspective;
};

out vec3 uv_pos;

void main()
{
    vec4 pos = perspective * view * vec4(position, 0.0);
    uv_pos = position;
    gl_Position = pos.xyww;
}
)SRC";

const char* skybox_fs = R"SRC(
#version 400 core

uniform samplerCube cubetex;

in vec3 uv_pos;

out vec4 FragColor;

void main()
{
    vec3 color = texture(cubetex, uv_pos).rgb;
    FragColor = vec4(color / (color + 1.0), 1);
}

)SRC";

const char* cubecvt_vs = R"SRC(
#version 400 core

layout (location = 0) in vec3 position;

uniform mat4 view;
uniform mat4 projection;

out vec3 uv_pos;

void main()
{
    mat4 v = mat4(mat3(view));
    uv_pos = position;
    gl_Position = projection * v * vec4(position, 1.0);
}
)SRC";

const char* cubecvt_fs = R"SRC(
#version 400 core

uniform sampler2D equi_map;

in vec3 uv_pos;

out vec4 FragColor;

// from learnopengl.com
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(uv_pos));
    FragColor = vec4(texture(equi_map, uv).rgb, 1);
}

)SRC";

GLuint Skybox::cube_vao = 0;

void Skybox::load_from_images(const std::vector<std::string>& images)
{
    assert(images.size() == 6 && "Wrong num image paths for skybox");
    assert(cubemap.get_id() != 0 && "cubemap tex didn't do its thing");

    cubemap.bind();
    int width, height, channels;
    for (u32 face = 0; face < CubemapFace_NFaces; face++)
    {
        u8* data = stbi_load(images[face].c_str(), &width, &height, &channels, 4);
        assert(data && "Couldn't load cubemap face");

        cubemap.buffer_face(face, width, height, data);
        stbi_image_free(data);
    }
    cubemap.unbind();

    shader.load_program(skybox_vs, skybox_fs);
}

void Skybox::load_from_dir(const std::string& dir)
{
    std::vector<std::string> images;
    images.reserve(6);
    images.push_back(dir + "/right.jpg");
    images.push_back(dir + "/left.jpg");
    images.push_back(dir + "/top.jpg");
    images.push_back(dir + "/bottom.jpg");
    images.push_back(dir + "/front.jpg");
    images.push_back(dir + "/back.jpg");

    load_from_images(images);
}


void Skybox::load_from_hdr(const std::string& hdr)
{
    assert(cubemap.get_id() != 0 && "Bad cubemap");

    int w, h, c;
    f32* data = stbi_loadf(hdr.c_str(), &w, &h, &c, 0);
    assert(data && "Bad hdr image");

    TextureBuilder builder;
    Texture hdr_tex = builder
        .with_internal_format(GL_RGB16F)
        .with_width(w).with_height(h)
        .with_src_format(GL_RGB)
        .with_data_type(GL_FLOAT)
        .with_data(data)
        .build();

    stbi_image_free(data);
    // convert from equirect to cubemap by rendering each cube face

    // right left top bottom front back
    //look_at(const Vec3& pos, const Vec3& target, Vec3 up = {0, 1, 0})
    sm::Mat4 views[6] = {
        sm::look_at(sm::Vec3{0, 0, 0}, sm::Vec3{1, 0, 0}, sm::Vec3{0, -1, 0}),
        sm::look_at(sm::Vec3{0, 0, 0}, sm::Vec3{-1, 0, 0}, sm::Vec3{0, -1, 0}),
        sm::look_at(sm::Vec3{0, 0, 0}, sm::Vec3{0, 1, 0}, sm::Vec3{0, 0, 1}),
        sm::look_at(sm::Vec3{0, 0, 0}, sm::Vec3{0, -1, 0}, sm::Vec3{0, 0, -1}),
        sm::look_at(sm::Vec3{0, 0, 0}, sm::Vec3{0, 0, 1}, sm::Vec3{0, -1, 0}),
        sm::look_at(sm::Vec3{0, 0, 0}, sm::Vec3{0, 0, -1}, sm::Vec3{0, -1, 0}),
    };
    sm::Mat4 persp = sm::mat4_I();
    persp[1][1] = -1;

    if (cube_vao == 0)
    {
        buffer_cube();
    }

    Framebuffer framebuf = Framebuffer::create_framebuffer(2048, 2048, 0);
    framebuf.bind();

    Shader cubemap_convert_shader;
    assert(cubemap_convert_shader.load_program(cubecvt_vs, cubecvt_fs) && "Shader failed");
    cubemap_convert_shader.use_program();
    cubemap_convert_shader.set_uniform_mat4("projection", persp);

    cubemap.bind();
    hdr_tex.bind_texture(GL_TEXTURE0);
    cubemap_convert_shader.set_uniform_int("equi_map", 0);
    glBindVertexArray(cube_vao);
    for (u32 face = 0; face < CubemapFace_NFaces; face++)
    {
        // TODO: cubemap class should be handled by texture
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                    0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);

        // TODO: better interface for the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemap.get_id(), 0);

        cubemap_convert_shader.set_uniform_mat4("view", views[face]);
        Renderer::set_clear_color(sm::Vec4{1, 1, 0, 1});
        framebuf.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    framebuf.unbind();
    glBindVertexArray(0);

    shader.load_program(skybox_vs, skybox_fs);
}

f32 cube_verts[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

i32 N_VERTS = 6 * 6;

void Skybox::buffer_cube()
{
    glGenVertexArrays(1, &cube_vao);

    u32 vbo;
    glGenBuffers(1, &vbo);

    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, N_VERTS * 3 * sizeof(f32), cube_verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Skybox::render()
{
    if (cube_vao == 0)
    {
        buffer_cube();
    }

    glBindVertexArray(cube_vao);

    glActiveTexture(GL_TEXTURE0);
    cubemap.bind();

    shader.use_program();
    shader.set_uniform_int("cubetex", 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    cubemap.unbind();
    glBindVertexArray(0);
}

} // namespace sr
