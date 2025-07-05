#ifndef VERTBUF_H_
#define VERTBUF_H_

#include <iostream>

// TODO: would be nice to decouple this
#include <glad/glad.h>

#include "spennytypes.h"

namespace sr
{

template <typename T, u32 n, u32 gl_type>
struct VertexComponent
{
    using Type = T;
    static constexpr u32 N = n;
    static constexpr u32 GLType = gl_type;
    static constexpr u32 Size = sizeof(T) * n;
};

template<u32 N>
using F32Component = VertexComponent<float, N, GL_FLOAT>;

template<u32 N>
using S32Component = VertexComponent<i32, N, GL_INT>;

template<u32 N>
using U32Component = VertexComponent<u32, N, GL_UNSIGNED_INT>;

template <typename... Components>
struct BufferLayout{};

template <typename L>
struct FirstComponent{};

template <typename First, typename... Rest>
struct FirstComponent<BufferLayout<First, Rest...>>
{
    using Type = First;
};

template <typename L>
struct Tail
{};

template <typename First, typename... Rest>
struct Tail<BufferLayout<First, Rest...>>
{
    using Type = BufferLayout<Rest...>;
};

template<typename Vert>
class VertexBuffer
{
    using Layout = typename Vert::Layout;
public:

    VertexBuffer()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        configure_attributes<Layout>();

        glBindVertexArray(0);
    }

    void buffer_data(const Vert* data, u64 n_verts, u32 mem_type = GL_STATIC_DRAW)
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, n_verts * sizeof(Vert), data, mem_type);

        glBindVertexArray(0);
    }

    void bind_vao()
    {
        glBindVertexArray(vao);
    }

    void unbind_vao()
    {
        glBindVertexArray(0);
    }

private:
    template<typename L>
    void configure_attributes(u32 attr_num = 0, u64 offset = 0)
    {
        if constexpr (std::is_same<BufferLayout<>, L>())
        {
            return;
        }
        else
        {
            using Attr = typename FirstComponent<L>::Type;
            u32 stride = sizeof(Vert);

            glVertexAttribPointer(attr_num,
                                  Attr::N,
                                  Attr::GLType,
                                  GL_FALSE,
                                  stride,
                                  (void*)offset);
            glEnableVertexAttribArray(attr_num);

            configure_attributes<typename Tail<L>::Type>(attr_num + 1, offset + Attr::Size);
        }
    }

    GLuint vao;
    GLuint vbo;
};

class IndexBuffer
{
public:
    IndexBuffer()
    {
        glGenBuffers(1, &ebo);
    }

    void buffer_indices(u32* indices, u64 count, u32 mem_type = GL_STATIC_DRAW)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices, mem_type);
        n_elems = count;
    }

    void bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    void unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    u64 get_n_elems() const noexcept
    {
        return n_elems;
    }

private:
    GLuint ebo;
    u64 n_elems;
};

template<typename Vertex>
struct IndexedGeometry
{
    GLuint prim_type;
    VertexBuffer<Vertex> vert_buf;
    IndexBuffer index_buf;
};

}

#endif // VERTBUF_H_
