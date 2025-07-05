#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <memory>
#include "renderer.h"

namespace sr
{

std::unique_ptr<Renderer> Renderer::renderer = nullptr;

Renderer::SDL::SDL(const std::string& win_title, u32 w, u32 h)
    : window(nullptr), renderer(nullptr), gl_context(nullptr)
{
    int gl_status = -1;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(win_title.c_str(), w, h, SDL_WINDOW_OPENGL);
    if (!window)
    {
        goto WindowErr;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        goto SDLErr;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        goto SDLErr;
    }

    gl_status = gladLoadGL();
    if (!gl_status)
    {
        goto GLErr;
    }
    std::cout << "Loaded GL version "
              << GLVersion.major << "." << GLVersion.minor << std::endl;

    return;

GLErr:
    std::cerr << "Failed to load GL" << std::endl;
SDLErr:
    SDL_DestroyWindow(window);
WindowErr:
    if (gl_status < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    }
    SDL_Quit();
    window = nullptr;
    renderer = nullptr;
    gl_context = nullptr;
}

Renderer::SDL::~SDL()
{
    if (gl_context)
    {
        SDL_GL_DestroyContext(gl_context);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

Renderer::Renderer(const std::string& win_title, u32 w, u32 h)
    : sdl(win_title, w, h)
      , default_framebuffer(nullptr)
      , camera_pos{2, 1, 2}
      , camera_target{0, 0.5, 0}
      , fovy{45.0}
      , near_clip{0.1}
      , far_clip{100.0}
      , global_ubo{0}
      , global_uniforms{0}
{
    assert(sdl.window && "SDL initialization failed");

    i32 out_w, out_h;
    SDL_GetRenderOutputSize(sdl.renderer, &out_w, &out_h);
    std::cout << "the screen is " << out_w << "x" << out_h << std::endl;

    default_framebuffer = std::unique_ptr<Framebuffer>(new Framebuffer);
    default_framebuffer->width = out_w;
    default_framebuffer->height = out_h;
    default_framebuffer->m_fbo = 0;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    global_uniforms.clip.x = out_w;
    global_uniforms.clip.y = out_h;
    global_uniforms.clip.z = near_clip;
    global_uniforms.clip.w = far_clip;
    global_uniforms.camera_position = sm::Vec4{camera_pos.x, camera_pos.y, camera_pos.z, 1};
    global_uniforms.view = sm::look_at(camera_pos, camera_target);
    auto aspect = out_w / out_h;
    global_uniforms.perspective = sm::perspective(fovy, aspect, near_clip, far_clip);

    glGenBuffers(1, &global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalUniforms), &global_uniforms, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::start(const std::string& win_title, u32 w, u32 h)
{
    assert(renderer == nullptr && "Cannot double-start renderer");
    renderer = std::unique_ptr<Renderer>(new Renderer(win_title, w, h));
}

void Renderer::end()
{
    if (renderer)
    {
        renderer.reset(nullptr);
    }
}

void Renderer::begin_frame()
{
    auto& framebuf = get_default_framebuffer();
    framebuf->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    get_renderer()->send_global_uniforms();
}

void Renderer::end_frame()
{
    SDL_GL_SwapWindow(get_renderer()->sdl.window);
}

std::unique_ptr<Renderer>& Renderer::get_renderer()
{
    assert(renderer != nullptr && "Tried to get renderer before it was started");
    return renderer;
}

std::unique_ptr<Framebuffer>& Renderer::get_default_framebuffer()
{
    return get_renderer()->default_framebuffer;
}

void Renderer::set_clear_color(sm::Vec4 rgba)
{
    glClearColor(rgba.r, rgba.g, rgba.b, rgba.a);
}

void Renderer::set_camera_position(sm::Vec3 camera_pos)
{
    get_renderer()->camera_pos = camera_pos;
}

void Renderer::set_camera_target(sm::Vec3 camera_target)
{
    get_renderer()->camera_target = camera_target;
}

void Renderer::set_fovy(f32 fovy)
{
    get_renderer()->fovy = fovy;
}

void Renderer::set_near_clip(f32 near_clip)
{
    get_renderer()->near_clip = near_clip;
}

void Renderer::set_far_clip(f32 far_clip)
{
    get_renderer()->far_clip = far_clip;
}

const Renderer::SDL& Renderer::get_sdl()
{
    return sdl;
}

void Renderer::send_global_uniforms()
{
    // TODO: Not this! We need to know dimensions of the framebuffer we're rendering to!
    auto& framebuf = get_default_framebuffer();
    auto aspect = framebuf->get_width() / framebuf->get_height();
    global_uniforms.clip.x = framebuf->get_width();
    global_uniforms.clip.y = framebuf->get_height();
    global_uniforms.clip.z = near_clip;
    global_uniforms.clip.w = far_clip;
    global_uniforms.camera_position = sm::Vec4{camera_pos.x, camera_pos.y, camera_pos.z, 1};
    global_uniforms.view = sm::look_at(camera_pos, camera_target);
    global_uniforms.perspective = sm::perspective(fovy, aspect, near_clip, far_clip);

    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalUniforms), &global_uniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::update_material_uniform(f32 roughness, f32 metalness)
{
    global_uniforms.material_properties.x = roughness;
    global_uniforms.material_properties.y = metalness;

    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(GlobalUniforms, material_properties), sizeof(sm::Vec4), &global_uniforms.material_properties);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

}
