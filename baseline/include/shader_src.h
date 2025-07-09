#ifndef SHADER_SRC_H_
#define SHADER_SRC_H_

const char* depth_prepass_vsrc = R"SRC(
#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

layout (std140) uniform GlobalUniforms
{
    vec4 clip;
    vec4 camera_pos;
    vec4 material_props;
    mat4 view;
    mat4 perspective;
};

uniform mat4 model_to_world;

void main()
{
    vec4 world_pos = model_to_world * vec4(position, 1.0);
    gl_Position = perspective * view * world_pos;
}
)SRC";

const char* vs_shader_src = R"SRC(
#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

layout (std140) uniform GlobalUniforms
{
    vec4 clip;
    vec4 camera_pos;
    vec4 material_props;
    mat4 view;
    mat4 perspective;
};

uniform mat4 model_to_world;

out vec2 tex;
out vec3 frag_world_pos;
out vec3 norm;
out mat3 tan_cob;

void main()
{
    vec4 world_pos = model_to_world * vec4(position, 1.0);
    gl_Position = perspective * view * world_pos;

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

layout (std140) uniform GlobalUniforms
{
    vec4 clip;
    vec4 camera_pos;
    vec4 material_props;
    mat4 view;
    mat4 perspective;
};

float pi = 3.14159;

uniform sampler2D teximg;
uniform sampler2D normals;

out vec4 FragColor;

float gsub(vec3 normal, vec3 dir, float k)
{
    float ndotd = max(dot(normal, dir), 0.0);
    float denom = ndotd * (1.0 - k) + k;
    return ndotd / denom;
}

float ggx(vec3 normal, vec3 view_dir, vec3 light_dir, float alpha)
{
    float k = ((alpha + 1) * (alpha + 1)) / 8.0;
    return gsub(normal, light_dir, k)* gsub(normal, view_dir, k);
}

float trggx(vec3 normal, vec3 half_dir, float alpha)
{
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
    vec3 light_color = vec3(4, 4, 3.4);

    vec3 light_dir_p = vec3(1.25, 1.25, 1.25);
    vec3 view_dir = normalize(vec3(camera_pos) - frag_world_pos);

    float dist = length(light_dir_p);
    vec3 light_dir = light_dir_p / dist;
    float atten = 1;// / (dist * dist);

    float roughness = material_props.x;
    float metalness = material_props.y;

    vec3 mapped_norm = vec3(0);
    if (material_props.z > 0)
    {
        vec3 sampled_norm = texture(normals, tex).rgb;
        sampled_norm = (sampled_norm * 2.0) - 1.0;
        mapped_norm = normalize(tan_cob * sampled_norm);
    }
    else
    {
        mapped_norm = normalize(norm);
    }

    vec3 half_dir = normalize(view_dir + light_dir);

    vec4 color = texture(teximg, tex);
    vec3 ambient = mix(vec3(0), color.xyz, 0.5);//vec3(0.3, 0.35, 0.4) * color.xyz;

    vec3 final_color = brdf(mapped_norm, light_dir, view_dir, half_dir, color.xyz, roughness, metalness)
        * (light_color * atten)
        * max(dot(light_dir, mapped_norm), 0)
        + ambient;

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


#endif // SHADER_SRC_H_
