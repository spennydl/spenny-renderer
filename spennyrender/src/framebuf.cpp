#include <glad/glad.h>

#include "framebuf.h"
#include "texture.h"

namespace sr
{

Framebuffer Framebuffer::create_framebuffer(u32 width, u32 height, u32 n_color_attachments, bool use_depth_attachement)
{
    Framebuffer result;
    glGenFramebuffers(1, &result.m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, result.m_fbo);

    for (u32 i = 0; i < n_color_attachments; i++)
    {
        TextureBuilder builder;
        Texture& tex = result.m_color_attachments[i];
        builder.with_width(width)
                .with_height(height)
                .with_internal_format(GL_RGBA)
                .with_src_format(GL_RGBA)
                .build_into(tex);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex.get_id(), 0);
    }

    if (use_depth_attachement)
    {
        TextureBuilder depth_builder;
        depth_builder.with_width(width)
                    .with_height(height)
                    .with_internal_format(GL_DEPTH_COMPONENT32F)
                    .with_src_format(GL_DEPTH_COMPONENT)
                    .with_data_type(GL_FLOAT)
                    .build_into(result.m_depth_attachment);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, result.m_depth_attachment.get_id(), 0);
    }

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer was NOT complete, it was " << status << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result.width = width;
    result.height = height;
    return result;
}

Framebuffer Framebuffer::create_framebuffer(u32 width, u32 height, u32 n_color_attachments, Texture depth_attachment)
{
    Framebuffer result;
    glGenFramebuffers(1, &result.m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, result.m_fbo);

    for (u32 i = 0; i < n_color_attachments; i++)
    {
        TextureBuilder builder;
        Texture& tex = result.m_color_attachments[i];
        builder.with_width(width)
                .with_height(height)
                .with_internal_format(GL_RGBA)
                .with_src_format(GL_RGBA)
                .build_into(tex);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex.get_id(), 0);
    }

    result.m_depth_attachment = depth_attachment;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, result.m_depth_attachment.get_id(), 0);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer was NOT complete, it was " << status << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result.width = width;
    result.height = height;
    return result;
}

void Framebuffer::clear(u32 what)
{
    glClear(what);
}

void Framebuffer::bind()
{
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Texture Framebuffer::get_depth_buffer()
{
    return m_depth_attachment;
}

Texture Framebuffer::get_color_attachment(u32 index)
{
    assert(index < 8);
    return m_color_attachments[index];
}

void Framebuffer::put_color_attachment(Texture attachment, u32 attachment_no, u32 type)
{
    assert(attachment_no < 8 && "Bad attachment number");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_no, type, attachment.get_id(), 0);
    m_color_attachments[attachment_no] = attachment;
}

}
