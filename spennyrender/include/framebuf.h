#ifndef FRAMEBUF_H_
#define FRAMEBUF_H_

#include <iostream>
#include <glad/glad.h>

#include "texture.h"

namespace sr
{

class Framebuffer
{
public:

    void bind();
    void unbind();
    void clear(u32 what = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Texture get_depth_buffer();
    Texture get_color_attachment(u32 index);

    void put_color_attachment(Texture attachment, u32 attachment_no, u32 type = GL_TEXTURE_2D);

    static Framebuffer create_framebuffer(u32 width, u32 height, u32 n_color_attachments, bool use_depth_attachement = true);
    static Framebuffer create_framebuffer(u32 width, u32 height, u32 n_color_attachments, Texture depth_attachment);

private:

    friend class Renderer;
    u32 width;
    u32 height;

    GLuint m_fbo;
    Texture m_depth_attachment;
    // could use GL_MAX_COLOR_ATTACHMENTS but I will likely never need that many
    Texture m_color_attachments[8];
};

} // namespace sr


#endif // FRAMEBUF_H_
