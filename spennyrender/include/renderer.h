#ifndef RENDERER_H_
#define RENDERER_H_

#include <iostream>
#include <memory>
#include <SDL3/SDL.h>

#include "framebuf.h"
#include "spennytypes.h"
#include "spennymath.h"
#include "vertbuf.h"

namespace sr
{

inline const char * GetGLErrorStr(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

inline void CheckGLError()
{
    while (true)
    {
        const GLenum err = glGetError();
        if (GL_NO_ERROR == err)
            break;

        std::cout << "GL Error: " << GetGLErrorStr(err) << std::endl;
    }
}

struct GlobalUniforms
{
    // x & y are screen dimensions, z & w are front/back clip
    sm::Vec4 clip;
    sm::Vec4 camera_position;

    // roughness, metallic, pad, pad
    sm::Vec4 material_properties;

    sm::Mat4 view;
    sm::Mat4 perspective;

    // TODO: lights here
};

class Renderer
{
public:

    static void start(const std::string& win_title, u32 w, u32 h);
    static void end();

    static void begin_frame();
    static void end_frame();
    static std::unique_ptr<Framebuffer>& get_default_framebuffer();
    static void set_clear_color(sm::Vec4 rgba);

    static void set_camera_position(sm::Vec3 camera_pos);
    static void set_camera_target(sm::Vec3 camera_target);
    static void set_fovy(f32 fovy);
    static void set_near_clip(f32 near_clip);
    static void set_far_clip(f32 far_clip);

    template<typename Vert>
    static void draw_indexed_geom(IndexedGeometry<Vert>& geom)
    {
        geom.vert_buf.bind_vao();
        glDrawElements(geom.prim_type, geom.index_buf.get_n_elems(), GL_UNSIGNED_INT, 0);
        geom.vert_buf.unbind_vao();
    }

    struct SDL
    {
        SDL(const std::string& win_title, u32 w, u32 h);
        ~SDL();

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_GLContext gl_context;
    };
    const SDL& get_sdl();
    static std::unique_ptr<Renderer>& get_renderer();

private:

    Renderer(const std::string& win_title, u32 w, u32 h);

    void send_global_uniforms();
    void update_material_uniform(f32 roughness, f32 metalness);

    SDL sdl;
    std::unique_ptr<Framebuffer> default_framebuffer;

    sm::Vec3 camera_pos;
    sm::Vec3 camera_target;

    f32 fovy;
    f32 near_clip;
    f32 far_clip;

    GLuint global_ubo;
    GlobalUniforms global_uniforms;

    static std::unique_ptr<Renderer> renderer;
};

}

#endif // RENDERER_H_
