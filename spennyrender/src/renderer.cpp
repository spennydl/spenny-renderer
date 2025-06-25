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
    : sdl(win_title, w, h), default_framebuffer(nullptr)
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
    get_default_framebuffer()->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

const Renderer::SDL& Renderer::get_sdl()
{
    return sdl;
}

}
