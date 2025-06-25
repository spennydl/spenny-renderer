#ifndef SPENNYMATH_H
#define SPENNYMATH_H

#include <iostream>
#include <cassert>
// TODO: cut this dep
#include <cmath>

#include "spennytypes.h"


namespace sm {

struct Vec2
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
        };
        struct
        {
            f32 u;
            f32 v;
        };
        f32 xy[2];
        f32 uv[2];
    };

    f32&
    operator[](usize i)
    {
        assert(i < 2 && "oob index");
        return xy[i];
    }

    const f32&
    operator[](usize i) const
    {
        assert(i < 2 && "oob index");
        return xy[i];
    }
};

struct Vec3
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };
        struct
        {
            f32 u;
            f32 v;
            f32 w;
        };
        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };
        f32 xyz[3];
        f32 uvw[3];
        f32 rgb[3];
    };

    f32&
    operator[](usize i)
    {
        assert(i < 3 && "oob index");
        return xyz[i];
    }

    const f32&
    operator[](usize i) const
    {
        assert(i < 3 && "oob index");
        return xyz[i];
    }
};

struct Vec4
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
        f32 xyzw[4];
        f32 rgba[4];
    };

    f32&
    operator[](usize i)
    {
        assert(i < 4 && "oob index");
        return xyzw[i];
    }

    const f32&
    operator[](usize i) const
    {
        assert(i < 4 && "oob index");
        return xyzw[i];
    }
};

inline Vec4
extend(Vec3 v, f32 w)
{
    return Vec4 { v.x, v.y, v.z, w };
}

inline Vec4
to_homog(Vec3 v)
{
    return extend(v, 1);
}

inline std::ostream&
operator<<(std::ostream& os, Vec3& v)
{
    for (int i = 0; i < 3; i++)
    {
        os << v.xyz[i] << "  ";
    }
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, Vec4& v)
{
    for (int i = 0; i < 4; i++)
    {
        os << v.xyzw[i] << "  ";
    }
    return os;
}

struct Mat4
{
    Vec4 cols[4];

    inline Vec4&
    operator[](usize i)
    {
        assert(i < 4 && "oob");
        return cols[i];
    }

    inline const Vec4&
    operator[](usize i) const
    {
        assert(i < 4 && "oob");
        return cols[i];
    }
};

inline Mat4
mat4_I()
{
    return Mat4
    {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    }};
}

inline Mat4
diag(f32 s)
{
    return Mat4
    {{
        {s, 0, 0, 0},
        {0, s, 0, 0},
        {0, 0, s, 0},
        {0, 0, 0, s},
    }};
}

inline Mat4
transpose(Mat4&& m)
{
    return Mat4
    {{
        {m[0].x, m[1].x, m[2].x, m[3].x},
        {m[0].y, m[1].y, m[2].y, m[3].y},
        {m[0].z, m[1].z, m[2].z, m[3].z},
        {m[0].w, m[1].w, m[2].w, m[3].w},
    }};
}

inline Mat4
from_rows(Vec4 r1, Vec4 r2, Vec4 r3, Vec4 r4)
{
    return Mat4
    {{
        {r1.x, r2.x, r3.x, r4.x},
        {r1.y, r2.y, r3.y, r4.y},
        {r1.z, r2.z, r3.z, r4.z},
        {r1.w, r2.w, r3.w, r4.w},
    }};
}

inline std::ostream&
operator<<(std::ostream& os, Mat4& mat)
{
    //Mat4 row_wise = transpose(mat);
    // this is an abysmal way to print
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            auto col = mat.cols[c];
            os << col[r] << " ";
        }
        os << std::endl;
    }

    //for (int i = 0; i < 4; i++)
    //{
        //os << row_wise.cols[i] << std::endl;
    //}
    return os;
}

inline Vec4
operator*(const Mat4& left, const Vec4& right)
{
    Vec4 result {0, 0, 0, 0};
    for (int n = 0; n < 4; n++)
    {
        for (int k = 0; k < 4; k++)
        {
            result.xyzw[n] += left.cols[n][k] * right[n];
        }
    }
    return result;
}

inline Mat4
operator*(const Mat4& left, const Mat4& right)
{
    Mat4 result = {0};
    for (int m = 0; m < 4; m++)
    {
        for (int n = 0; n < 4; n++)
        {
            for (int k = 0; k < 4; k++)
            {
                result[m][n] += left[k][m] * right[n][k];
            }
        }
    }
    return result;
}

inline Mat4
operator*(const Mat4& left, const f32& right)
{
    Mat4 result = left;
    for (i32 n = 0; n < 4; n++)
    {
        for (i32 m = 0; m < 4; m++)
        {
            result[m][n] = left[m][n] * right;
        }
    }
    return result;
}

inline Mat4
translation_by(Vec3 by)
{
    Mat4 result = mat4_I();

    result.cols[3] = Vec4 { by.x, by.y, by.x, 1.0 };

    return result;
}

inline Mat4
scale_by(f32 by)
{
    Mat4 result = diag(by);

    result[3][3] = 1;

    return result;
}

inline Mat4
scale_by(Vec3 by)
{
    Mat4 result = mat4_I();

    result[0][0] = by.x;
    result[1][1] = by.y;
    result[2][2] = by.z;
    result[3][3] = 1;

    return result;
}

inline Mat4
rotate(f32 pitch, f32 yaw, f32 roll)
{
    const float DEG2RAD = acos(-1.0f) / 180;
    f32 sinp = sin(DEG2RAD * pitch);
    f32 cosp = cos(DEG2RAD * pitch);
    f32 siny = sin(DEG2RAD * yaw);
    f32 cosy = cos(DEG2RAD * yaw);
    f32 sinr = sin(DEG2RAD * roll);
    f32 cosr = cos(DEG2RAD * roll);

    Mat4 pitch_m = mat4_I();
    pitch_m[1][1] = cosp;
    pitch_m[1][2] = sinp;
    pitch_m[2][1] = -sinp;
    pitch_m[2][2] = cosp;

    Mat4 yaw_m = mat4_I();
    yaw_m[0][0] = cosy;
    yaw_m[2][0] = siny;
    yaw_m[0][2] = -siny;
    yaw_m[2][2] = cosy;

    Mat4 roll_m = mat4_I();
    roll_m[0][0] = cosr;
    roll_m[1][0] = -sinr;
    roll_m[0][1] = sinr;
    roll_m[1][1] = cosr;

    return yaw_m * roll_m * pitch_m;
}

//--------------------------------------------------------------------------------
// Vec2 Operators
//--------------------------------------------------------------------------------

// Addition
inline Vec2
operator+(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2 { lhs.x + rhs.x, lhs.y + rhs.y };
}

inline Vec2
operator+(const Vec2& lhs, const f32 rhs)
{
    return Vec2 { lhs.x + rhs, lhs.y + rhs };
}

inline Vec2
operator+(const f32 lhs, const Vec2 rhs)
{
    return rhs + lhs;
}

// Subtraction
inline Vec2
operator-(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2 { lhs.x - rhs.x, lhs.y - rhs.y };
}

inline Vec2
operator-(const Vec2& lhs, const f32& rhs)
{
    return Vec2 { lhs.x - rhs, lhs.y - rhs };
}

inline Vec2
operator-(const f32& lhs, const Vec2& rhs)
{
    return Vec2 { lhs - rhs.x, lhs - rhs.y };
}

// Multiplication by a scalar. NO vector/vector mult,
// use a function to be clear what product you want
inline Vec2
operator*(const Vec2& lhs, const f32& rhs)
{
    return Vec2 { lhs.x * rhs, lhs.y * rhs };
}

inline Vec2
operator*(const f32& lhs, const Vec2& rhs)
{
    return rhs * lhs;
}

inline Vec2
operator/(const Vec2& lhs, const f32& rhs)
{
    return Vec2 { lhs.x / rhs, lhs.y / rhs};
}

inline bool
operator==(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

//--------------------------------------------------------------------------------
// Vec3 Operators
//--------------------------------------------------------------------------------

// Addition
inline Vec3
operator+(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3 { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline Vec3
operator+(const Vec3& lhs, const f32 rhs)
{
    return Vec3 { lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
}

inline Vec3
operator+(const f32 lhs, const Vec3 rhs)
{
    return rhs + lhs;
}

// Subtraction
inline Vec3
operator-(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3 { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

inline Vec3
operator-(const Vec3& lhs, const f32& rhs)
{
    return Vec3 { lhs.x - rhs, lhs.y - rhs, lhs.z - rhs };
}

inline Vec3
operator-(const f32& lhs, const Vec3& rhs)
{
    return Vec3 { lhs - rhs.x, lhs - rhs.y, lhs - rhs.z };
}

// Multiplication by a scalar. NO vector/vector mult,
// use a function to be clear what product you want
inline Vec3
operator*(const Vec3& lhs, const f32& rhs)
{
    return Vec3 { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

inline Vec3
operator*(const f32& lhs, const Vec3& rhs)
{
    return rhs * lhs;
}

inline Vec3
operator/(const Vec3& lhs, const f32& rhs)
{
    return Vec3 { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

inline bool
operator==(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

//--------------------------------------------------------------------------------
// Vec4 Operators
//--------------------------------------------------------------------------------

// Addition
inline Vec4
operator+(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4 { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
}

inline Vec4
operator+(const Vec4& lhs, const f32 rhs)
{
    return Vec4 { lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs };
}

inline Vec4
operator+(const f32 lhs, const Vec4 rhs)
{
    return rhs + lhs;
}

// Subtraction
inline Vec4
operator-(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4 { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
}

inline Vec4
operator-(const Vec4& lhs, const f32& rhs)
{
    return Vec4 { lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs };
}

inline Vec4
operator-(const f32& lhs, const Vec4& rhs)
{
    return Vec4 { lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w };
}

// Multiplication by a scalar. NO vector/vector mult,
// use a function to be clear what product you want
inline Vec4
operator*(const Vec4& lhs, const f32& rhs)
{
    return Vec4 { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
}

inline Vec4
operator*(const f32& lhs, const Vec4& rhs)
{
    return rhs * lhs;
}

inline Vec4
operator/(const Vec4& lhs, const f32& rhs)
{
    return Vec4 { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs};
}

inline f32
dot(const Vec2& lhs, const Vec2& rhs)
{
    return (lhs.x * rhs.x) + (lhs.y * rhs.y);
}

inline f32
dot(const Vec3& lhs, const Vec3& rhs)
{
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

inline f32
dot3(const Vec4& lhs, const Vec4& rhs)
{
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

inline f32
dot(const Vec4& lhs, const Vec4& rhs)
{
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);
}

inline Vec2
hadamard(const Vec2& lhs, const Vec3& rhs)
{
    return Vec2 { lhs.x * rhs.x, lhs.y * rhs.y };
}

inline Vec3
hadamard(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3 { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

inline Vec4
hadamard(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4 { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w };
}


inline Vec3
cross(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3 { (lhs.y * rhs.z) - (lhs.z * rhs.y),
                  -((lhs.x * rhs.z) - (lhs.z * rhs.x)),
                  (lhs.x * rhs.y) - (lhs.y * rhs.x)};
}

inline Vec4
cross3(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4 { (lhs.y * rhs.z) - (lhs.z * rhs.y),
                  -((lhs.x * rhs.z) - (lhs.z * rhs.x)),
                  (lhs.x * rhs.y) - (lhs.y * rhs.x),
                  1.0f };
}

inline f32
length(Vec2 v)
{
    return sqrt(dot(v, v));
}

inline f32
length(Vec3 v)
{
    return sqrt(dot(v, v));
}

inline f32
length(Vec4 v)
{
    return sqrt(dot(v, v));
}

inline Vec3
norm(Vec3 v)
{
    f32 l = length(v);
    assert(l > 0 && "kablam");
    return v / l;
}

inline Vec4
norm3(Vec4 v)
{
    Vec3 comps = {v.x, v.y, v.z};
    f32 l = length(comps);
    assert(l > 0 && "kablam");
    return Vec4 { v.x / l, v.y / l, v.z / l, 1.0f };
}

inline Vec4
norm(Vec4 v)
{
    f32 l = length(v);
    assert(l > 0 && "kablam");
    return v / l;
}


inline f32
signof_zero(f32 val)
{
    return (val < 0) ? -1.0 : (val == 0) ? 0 : 1;
}

inline f32
signof(f32 val)
{
    return (val < 0) ? -1.0f : 1.0f;
}

inline f32
abs(f32 val)
{
    return val * signof(val);
}

inline f32
floor(f32 val)
{
    return (f32)((i32)val);
}

inline Vec2
floor(Vec2 val)
{
    return {floor(val.x), floor(val.y)};
}

inline Vec3
floor(Vec3 val)
{
    return {floor(val.x), floor(val.y), floor(val.z)};
}
inline Vec4
floor(Vec4 val)
{
    return {floor(val.x), floor(val.y), floor(val.z), floor(val.w)};
}

inline f32
fract(f32 val)
{
    return val - floor(val);
}

inline Vec2
fract(Vec2 val)
{
    return {fract(val.x), fract(val.y)};
}

inline Vec3
fract(Vec3 val)
{
    return {fract(val.x), fract(val.y), fract(val.z)};
}
inline Vec4
fract(Vec4 val)
{
    return {fract(val.x), fract(val.y), fract(val.z), fract(val.w)};
}

inline f32
ceil(f32 val)
{
    return floor(val) + signof_zero(fract(val));
}

inline f32
round(f32 val)
{
    f32 fval = floor(val);
    return fval;
    f32 dist_to_floor = val - fval;
    return (dist_to_floor >= 0.5) ? fval + signof(val) : fval;
}

// 3D camera stuff

inline Mat4
look_at(const Vec3& pos, const Vec3& target, Vec3 up = {0, 1, 0})
{
    auto view_dir = norm(target - pos);

    if (up == Vec3{0, 1, 0} && view_dir.x == 0 && view_dir.z == 0)
    {
        up.y = 0;
        if (view_dir.y < 0)
        {
            up.z = -1;
        }
        else
        {
            up.z = 1;
        }
    }

    auto right = norm(cross(view_dir, up));
    auto real_up = norm(cross(right, view_dir));

    Mat4 pmat = mat4_I();
    pmat[0][3] = -pos.x;
    pmat[1][3] = -pos.y;
    pmat[2][3] = -pos.z;

    Mat4 basis = { extend(right, 0),
                   extend(real_up, 0),
                   extend(-1.0f * view_dir, 0),
                   Vec4{0, 0, 0, 1} };

    return pmat * basis;
}

inline Mat4
perspective(const f32 fovy, const f32 aspect_ratio, const f32 front = 0.1, const f32 back = 10.0)
{
    const float DEG2RAD = acos(-1.0f) / 180;

    float tangent = tan(fovy/2 * DEG2RAD);    // tangent of half fovY
    float top = front * tangent;              // half height of near plane
    float right = top * aspect_ratio;          // half width of near plane

    // params: left, right, bottom, top, near(front), far(back)
    Mat4 matrix = {0};
    matrix[0][0]  =  front / right;
    matrix[1][1]  =  front / top;
    matrix[2][2] = -(back + front) / (back - front);
    matrix[2][3] = -(2 * back * front) / (back - front);
    matrix[3][2] = -1;
    matrix[3][3] =  0;
    return matrix;
}

inline u64
dbj2(const char* str, usize len)
{
    u64 hash = 5381;
    for (usize i = 0; i < len && str[i]; i++)
    {
        hash = ((hash << 5) + hash) * str[i];
    }
    return hash;
}

} // namespace sm

#endif // SPENNYMATH_H
