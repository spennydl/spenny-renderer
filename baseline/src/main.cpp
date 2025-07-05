#include <iostream>
#include <memory>
#include <string>
#include <glad/glad.h>
#include <vector>

#include "vertbuf.h"
#include "spennymath.h"
#include "framebuf.h"
#include "shader.h"
#include "texture.h"
#include "model.h"
#include "renderer.h"

#include "shader_src.h"

#ifndef BASELINE_RESOURCE_DIR
#define BASELINE_RESOURCE_DIR "./"
#endif

struct SimpleVertex
{
    using Layout = sr::BufferLayout<sr::F32Component<3>,
                                    sr::F32Component<2>>;
    sm::Vec3 pos;
    sm::Vec2 uv;
};

SimpleVertex screen_quad[] = {
    {{-1.0, -1.0, 0.0}, {0.0, 0.0}},
    {{1.0, -1.0, 0.0},  {1.0, 0.0}},
    {{-1.0, 1.0, 0.0},  {0.0, 1.0}},
    {{1.0, 1.0, 0.0},   {1.0, 1.0}},
};

u32 idxs[] = { 0, 1, 2, 1, 2, 3 };

auto buffer_quad() -> sr::IndexedGeometry<SimpleVertex>
{
    sr::IndexedGeometry<SimpleVertex> result;
    result.vert_buf.buffer_data(screen_quad, 4);

    result.vert_buf.bind_vao();
    result.index_buf.bind();
    result.index_buf.buffer_indices(idxs, 6);

    result.prim_type = GL_TRIANGLES;

    result.vert_buf.unbind_vao();
    return result;
}

auto main(void) -> int
{
    sr::Renderer::start("spenny renderer", 1280, 720);

    using VertBuf = sr::VertexBuffer<sr::Vertex>;
    using IndexedGeom = sr::IndexedGeometry<sr::Vertex>;

    SDL_Event e;
    bool running = true;

    sr::Shader depth_prepass;
    if (!depth_prepass.load_program(depth_prepass_vsrc, fs_shader_empty_src))
    {
        std::cout << "depth" << std::endl;
        return 1;
    }

    sr::Shader shader;
    if (!shader.load_program(vs_shader_src, fs_shader_src))
    {
        std::cout << "pbr" << std::endl;
        return 1;
    }

    sr::Shader screen_shader;
    if (!screen_shader.load_program(simple_quad_vsrc, simple_quad_fsrc))
    {
        std::cout << "screen" << std::endl;
        return 1;
    }

    sr::ModelLoader model_loader;
    auto maybe_model = model_loader.load_from_file(BASELINE_RESOURCE_DIR "/testarena/testlevel.glb");
    if (!maybe_model)
    {
        return 1;
    }
    auto model = *maybe_model;
    std::cout << "Loaded " << model.meshes.size() << " meshes" << std::endl;

    auto quad = buffer_quad();

    sr::Skybox skybox;
    skybox.load_from_dir(BASELINE_RESOURCE_DIR "/skybox");

    sr::Skybox hdr_skybox;
    hdr_skybox.load_from_hdr(BASELINE_RESOURCE_DIR "/hdr/skycloudy/HDR_029_Sky_Cloudy_Ref.hdr");

    std::vector<IndexedGeom> buffers;
    std::vector<u32> mat_idxs;

    for (auto& mesh : model.meshes)
    {
        GLuint ebo;

        IndexedGeom geometry;
        geometry.prim_type = GL_TRIANGLES;

        geometry.vert_buf.buffer_data(mesh.verts.data(), mesh.verts.size());
        geometry.vert_buf.bind_vao();

        geometry.index_buf.bind();
        geometry.index_buf.buffer_indices(mesh.indices.data(), mesh.indices.size());

        geometry.vert_buf.unbind_vao();
        buffers.push_back(geometry);
        mat_idxs.push_back(mesh.material_index);
    }

    auto model_to_world = sm::scale_by(sm::Vec3{50, 1, 50});

    sr::Framebuffer depth_buffer = sr::Framebuffer::create_framebuffer(1280, 720, 0, true);
    sr::Framebuffer render_buffer = sr::Framebuffer::create_framebuffer(1280, 720, 1, depth_buffer.get_depth_buffer());

    i64 ticks = SDL_GetTicks();
    i64 last_ticks = ticks;
    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_EVENT_QUIT:
                {
                    running = false;
                } break;
                default:
                    break;
            }
        }

        i64 ticks = SDL_GetTicks();
        i64 ticks_delta = ticks - last_ticks;

        f32 time_s = ticks / 1000.0f;
        f32 dt = ticks_delta / 1000.0f;

        sm::Vec4 camera_pos_h = sm::Vec4{2, 1, 2, 1};
        sm::Mat4 rot = sm::rotate(0, 36.0 * dt, 0);

        camera_pos_h = rot * camera_pos_h;
        sm::Vec3 camera_pos{ camera_pos_h.x, camera_pos_h.y, camera_pos_h.z };

        sr::Renderer::set_camera_position(camera_pos);

        sr::Renderer::begin_frame();
        // depth prepass
        depth_buffer.bind();
        depth_buffer.clear(GL_DEPTH_BUFFER_BIT);
        depth_prepass.use_program();
        depth_prepass.set_uniform_mat4("model_to_world", model_to_world);
        for (auto& buffer : buffers)
        {
            sr::Renderer::draw_indexed_geom(buffer);
        }
        depth_buffer.unbind();

        // render pass
        render_buffer.bind();

        sr::Renderer::set_clear_color(sm::Vec4{0.071, 0.071, 0.071, 1.0});
        render_buffer.clear(GL_COLOR_BUFFER_BIT);

        shader.use_program();
        shader.set_uniform_mat4("model_to_world", model_to_world);
        shader.set_uniform_int("texture", 0);
        shader.set_uniform_int("normals", 1);

        for (u32 i = 0; i < buffers.size(); i++)
        {
            auto mat = model.materials[mat_idxs[i]];
            auto diffuse = mat.diffuse;
            auto normals = mat.normals;
            diffuse.bind_texture(GL_TEXTURE0);
            normals.bind_texture(GL_TEXTURE1);

            sr::Renderer::draw_indexed_geom(buffers[i]);
        }

        hdr_skybox.render();

        render_buffer.unbind();
        sr::Renderer::get_default_framebuffer()->bind();

        render_buffer.get_color_attachment(0).bind_texture(GL_TEXTURE0);
        screen_shader.use_program();
        screen_shader.set_uniform_int("scene", GL_TEXTURE0);

        sr::Renderer::draw_indexed_geom(quad);

        sr::Renderer::end_frame();

    }

    sr::Renderer::end();
    return 0;
}
