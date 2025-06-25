#include <string>
#include <glad/glad.h>

#include "shader.h"

namespace sr
{

bool
check_shader_status(GLuint id, bool prog)
{
    int success;
    char infoLog[512];
    if (prog) {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
    } else {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    }
    if (!success) {
        if (prog) {
            glGetProgramInfoLog(id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::LINK_FAIL\n" << glGetError() << std::endl << infoLog << std::endl;
        } else {
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMP_FAILED\n" << infoLog << std::endl;
        }
    }
    return success;
}

bool Shader::load_program(const std::string& vs_src, const std::string& fs_src)
{
    auto vs = glCreateShader(GL_VERTEX_SHADER);

    const char *vs_csrc = vs_src.c_str();
    const char *fs_csrc = fs_src.c_str();
    glShaderSource(vs, 1, &vs_csrc, nullptr);
    glCompileShader(vs);
    if (!check_shader_status(vs, false)) {
        std::cerr << "vertex failed" << std::endl;
        return false;
    }

    auto fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_csrc, nullptr);
    glCompileShader(fs);
    if (!check_shader_status(fs, false)) {
        std::cerr << "frag failed" << std::endl;
        return false;
    }

    auto shader_id = glCreateProgram();
    glAttachShader(shader_id, vs);
    glAttachShader(shader_id, fs);
    glLinkProgram(shader_id);
    if (!check_shader_status(shader_id, true)) {
        std::cerr << "link failed" << std::endl;
        return false;
    }

    this->id = shader_id;
    return true;
}

void Shader::use_program()
{
    glUseProgram(this->id);
}

void Shader::set_uniform_mat4(const std::string& name, const sm::Mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(this->id, name.c_str()),
                        1,
                        GL_FALSE,
                        reinterpret_cast<const f32*>(&mat));
}

void Shader::set_uniform_vec3(const std::string& name, const sm::Vec3& v3)
{
    glUniform3fv(glGetUniformLocation(this->id, name.c_str()),
                                      1, reinterpret_cast<const f32*>(&v3));
}

void Shader::set_uniform_int(const std::string& name, const i32 val)
{
    auto loc = glGetUniformLocation(this->id, name.c_str());
    glUniform1i(loc, val);
}

void Shader::set_uniform_float(const std::string& name, const f32 val)
{
    glUniform1f(glGetUniformLocation(this->id, name.c_str()), val);
}

} // namespace sr
