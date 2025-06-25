#ifndef SPENNY_SHADER_H
#define SPENNY_SHADER_H

#include <string>
#include <glad/glad.h>

#include "spennymath.h"

namespace sr
{

class Shader
{
public:
    bool load_program(const std::string& vs_src, const std::string& fs_src);

    void use_program();

    void set_uniform_mat4(const std::string& name, const sm::Mat4& mat);
    void set_uniform_vec3(const std::string& name, const sm::Vec3& v3);
    void set_uniform_int(const std::string& name, const i32 val);
    void set_uniform_float(const std::string& name, const f32 val);

    u32 get_id() const { return id; }

private:
    GLuint id;
};

} // namespace sr


#endif // SPENNY_SHADER_H
