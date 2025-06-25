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

#ifndef BASELINE_RESOURCE_DIR
#define BASELINE_RESOURCE_DIR "./"
#endif

f32 screen_quad[] = {
    -1.0, -1.0, 0.0, 0.0, 0.0,
    1.0, -1.0, 0.0, 1.0, 0.0,
    -1.0, 1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 0.0, 1.0, 1.0,
};

u32 idxs[] = { 0, 1, 2, 1, 2, 3 };

auto buffer_quad() -> GLuint
{
    u32 vao, vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 20, screen_quad, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * 6, idxs, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return vao;
}

const char* depth_prepass_vsrc = R"SRC(
#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

uniform mat4 model_to_world;
uniform mat4 world_to_view;
uniform mat4 projection;

void main()
{
    vec4 world_pos = model_to_world * vec4(position, 1.0);
    gl_Position = projection * world_to_view * world_pos;
}
)SRC";

const char* vs_shader_src = R"SRC(
#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

uniform mat4 model_to_world;
uniform mat4 world_to_view;
uniform mat4 projection;

out vec2 tex;
out vec3 frag_world_pos;
out vec3 norm;
out mat3 tan_cob;

void main()
{
    vec4 world_pos = model_to_world * vec4(position, 1.0);
    gl_Position = projection * world_to_view * world_pos;

    tex = uv;
    frag_world_pos = world_pos.xyz;

    mat3 normal_mat = transpose(inverse(mat3(model_to_world)));

    vec3 transform_norm = normalize(normal_mat * normal);
    vec3 transform_tan = normalize(normal_mat * tangent);
    vec3 transform_bitan = normalize(normal_mat * bitangent);

    tan_cob = mat3(transform_tan, transform_bitan, transform_norm);
    norm = transform_norm;
}
)SRC";

const char* fs_shader_empty_src = R"SRC(
#version 400 core

void main() {}
)SRC";


const char* fs_shader_src = R"SRC(
#version 400 core

in vec2 tex;
in vec3 frag_world_pos;
in mat3 tan_cob;
in vec3 norm;

float pi = 3.14159;

uniform vec3 camera_pos;
uniform sampler2D teximg;
uniform sampler2D normals;
uniform float t;

out vec4 FragColor;

float gsub(vec3 normal, vec3 dir, float k)
{
    float ndotd = max(dot(normal, dir), 0);
    float denom = ndotd * (1-k) + k;
    return ndotd / denom;
}

float ggx(vec3 normal, vec3 view_dir, vec3 light_dir, float alpha)
{
    float k = ((alpha + 1) * (alpha + 1)) / 8.0;
    return gsub(normal, light_dir, k) * gsub(normal, view_dir, k);
}

float trggx(vec3 normal, vec3 light_dir, float alpha)
{
    vec3 half_dir = normalize(normal + light_dir);
    float a2 = alpha * alpha * alpha * alpha;
    float ndoth = max(dot(normal, half_dir), 0);
    float inner_denom = ((ndoth * ndoth) * (a2 - 1.0)) + 1.0;
    return a2 / (pi * (inner_denom * inner_denom));
}

vec3 fresn(float cosT, vec3 color, float metalness)
{
    vec3 f0 = mix(vec3(0.04), color, metalness);
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosT, 0, 1), 5.0);
}

vec3 brdf(vec3 normal, vec3 light_dir, vec3 view_dir, vec3 half_dir, vec3 albedo, float alpha, float metalness)
{
    vec3 lambert = vec3(albedo / pi);

    float d = trggx(normal, half_dir, alpha);
    float g = ggx(normal, view_dir, light_dir, alpha);
    float cosT = max(dot(half_dir, view_dir), 0);
    vec3 f = fresn(cosT, albedo, metalness);
    vec3 num = d * f * g;
    float denom = 4 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);

    vec3 kd = (1 - metalness) * (vec3(1.0) - f);

    return kd * lambert + (num / (denom + 0.0001));
}

void main()
{
    vec3 light_color = vec3(14, 14, 9.3);
    //float lightx = cos(t) * 3;
    //float lightz = sin(t) * 10;
    //vec3 light_pos = vec3(lightx, lightz, 2);

    vec3 light_dir_p = vec3(-20, 20, -20);//light_pos - frag_world_pos;
    vec3 view_dir = normalize(camera_pos - frag_world_pos);

    float dist = length(light_dir_p);
    vec3 light_dir = light_dir_p / dist;
    float atten = 1;// / dist;

    float alpha = 0.6;
    float metalness = 0.0;
    vec4 color = texture(teximg, tex);
    vec3 ambient = vec3(0.0, 0.1, 0.2) * color.xyz;

    vec3 sampled_norm = texture(normals, tex).rgb;
    sampled_norm = (sampled_norm * 2.0) - 1.0;

    vec3 half_dir = normalize(view_dir + light_dir);

    vec3 mapped_norm = normalize(tan_cob * sampled_norm);

    vec3 final_color = brdf(mapped_norm, light_dir, view_dir, half_dir, color.xyz, alpha, metalness) * (light_color * atten) * max(dot(light_dir, mapped_norm), 0) + ambient;

    float exposure = 0.7;
    final_color = vec3(1.0) - exp(-final_color * exposure);

    //final_color = final_color / (final_color + vec3(1.0));
    final_color = pow(final_color, vec3(1.0/2.2));

    FragColor = vec4(final_color, color.w);
}
)SRC";

const char* simple_quad_vsrc = R"SRC(
#version 400 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec2 tex_pos;

out vec2 UV;

void main()
{
    gl_Position = vec4(vert_pos, 1);
    UV = tex_pos;
}
)SRC";

const char* simple_quad_fsrc = R"SRC(
#version 400 core

in vec2 UV;

out vec4 FragColor;
uniform sampler2D scene;

float near = 0.01;
float far = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    //float depth_val = LinearizeDepth(texture(depth_attachment, UV).r) / far;
    //FragColor = vec4(depth_val, depth_val, depth_val, 1);
    FragColor = texture(scene, UV);
}
)SRC";

auto main(void) -> int
{
    sr::Renderer::start("spenny renderer", 1280, 720);

    using VertBuf = sr::VertexBuffer<sr::Vertex>;

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
    auto maybe_model = model_loader.load_from_file(BASELINE_RESOURCE_DIR "/fox/fox.glb");
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

    std::vector<VertBuf> buffers;
    std::vector<u32> mesh_elems;
    std::vector<u32> mat_idxs;

    for (auto& mesh : model.meshes)
    {
        GLuint ebo;

        VertBuf buffer;

        buffer.buffer_data(mesh.verts.data(), mesh.verts.size());

        buffer.bind_vao();

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(u32), mesh.indices.data(), GL_STATIC_DRAW);

        buffer.unbind_vao();
        buffers.push_back(buffer);
        mesh_elems.push_back(mesh.indices.size());
        mat_idxs.push_back(mesh.material_index);
    }

    auto model_to_world = sm::rotate(0, 180, 0);
    auto projection = sm::transpose(sm::perspective(45, 1280.0 / 720.0));

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

        auto world_to_view = sm::look_at(camera_pos, sm::Vec3 { 0, 0.5, 0 });

        sr::Renderer::begin_frame();

        // depth prepass
        depth_buffer.bind();
        depth_buffer.clear(GL_DEPTH_BUFFER_BIT);
        depth_prepass.use_program();
        depth_prepass.set_uniform_mat4("model_to_world", model_to_world);
        depth_prepass.set_uniform_mat4("world_to_view", world_to_view);
        depth_prepass.set_uniform_mat4("projection", projection);
        for (u32 i = 0; i < buffers.size(); i++)
        {
            buffers[i].bind_vao();
            glDrawElements(GL_TRIANGLES, mesh_elems[i], GL_UNSIGNED_INT, 0);
        }
        depth_buffer.unbind();

        // render pass
        render_buffer.bind();

        sr::Renderer::set_clear_color(sm::Vec4{0.071, 0.071, 0.071, 1.0});
        render_buffer.clear(GL_COLOR_BUFFER_BIT);

        shader.use_program();
        shader.set_uniform_mat4("model_to_world", model_to_world);
        shader.set_uniform_mat4("world_to_view", world_to_view);
        shader.set_uniform_mat4("projection", projection);
        shader.set_uniform_vec3("camera_pos", camera_pos);
        shader.set_uniform_int("texture", 0);
        shader.set_uniform_int("normals", 1);
        shader.set_uniform_float("t", time_s);

        for (u32 i = 0; i < buffers.size(); i++)
        {
            auto mat = model.materials[mat_idxs[i]];
            auto diffuse = mat.diffuse;
            auto normals = mat.normals;
            diffuse.bind_texture(GL_TEXTURE0);
            normals.bind_texture(GL_TEXTURE1);
            buffers[i].bind_vao();
            glDrawElements(GL_TRIANGLES, mesh_elems[i], GL_UNSIGNED_INT, 0);
        }

        hdr_skybox.render(world_to_view, projection);

        render_buffer.unbind();
        sr::Renderer::get_default_framebuffer()->bind();

        render_buffer.get_color_attachment(0).bind_texture(GL_TEXTURE0);
        screen_shader.use_program();
        screen_shader.set_uniform_int("scene", GL_TEXTURE0);
        glBindVertexArray(quad);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        sr::Renderer::end_frame();

    }

    sr::Renderer::end();
    return 0;
}
