#ifndef RENDERER_H_
#define RENDERER_H_

#include <memory>
#include <SDL3/SDL.h>

#include "framebuf.h"
#include "spennytypes.h"
#include "spennymath.h"

namespace sr
{

class Renderer
{
public:

    static void start(const std::string& win_title, u32 w, u32 h);
    static void end();

    static void begin_frame();
    static void end_frame();
    static std::unique_ptr<Framebuffer>& get_default_framebuffer();
    static void set_clear_color(sm::Vec4 rgba);

    static std::unique_ptr<Renderer>& get_renderer();

    struct SDL
    {
        SDL(const std::string& win_title, u32 w, u32 h);
        ~SDL();

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_GLContext gl_context;
    };
    const SDL& get_sdl();

private:

    Renderer(const std::string& win_title, u32 w, u32 h);

    SDL sdl;
    std::unique_ptr<Framebuffer> default_framebuffer;

    static std::unique_ptr<Renderer> renderer;
};

}

#endif // RENDERER_H_
