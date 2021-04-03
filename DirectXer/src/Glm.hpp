#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_SSE2
#define GLM_FORCE_AVX

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vec_swizzle.hpp>



inline glm::mat4 init_rotation(glm::vec3 forward, glm::vec3 up, glm::vec3 right)
{
    glm::mat4 m;

    m[0][0] = right.x;
    m[0][1] = right.y;
    m[0][2] = right.z;
    m[0][3] = 0;
    m[1][0] = up.x;
    m[1][1] = up.y;
    m[1][2] = up.z;
    m[1][3] = 0;
    m[2][0] = forward.x;
    m[2][1] = forward.y;
    m[2][2] = forward.z;
    m[2][3] = 0;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;

    return m;
}

inline glm::mat4 init_rotation(glm::vec3 forward, glm::vec3 up)
{
    auto f = glm::normalize(forward);
    auto r = glm::cross(f, glm::normalize(up));
    auto u = glm::cross(f, r);
    return init_rotation(f, u, r);
}

inline glm::mat4 init_rotation(float angle, glm::vec3 axis)
{
    return glm::rotate(angle, axis);
}

inline glm::mat4 init_scale(float x, float y, float z)
{

    return glm::mat4(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
}

inline glm::mat4 init_scale(glm::vec3 v)
{
    return init_scale(v.x, v.y, v.z);
}

inline glm::mat4 init_translate(float x, float y, float z)
{

    return glm::mat4(1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1);
}

inline glm::mat4 init_translate(glm::vec3 v)
{
    return init_translate(v.x, v.y, v.z);
}

inline glm::mat4 init_identity()
{

    return glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

inline glm::vec3 rotate(glm::vec3 v, glm::quat q)
{
    // return glm::conjugate(q) * v * q;

    auto res = (q * glm::quat(0, v.x, v.y, v.z)) * glm::conjugate(q);
    return glm::vec3(res.x, res.y, res.z);
}

inline glm::vec3 up(glm::quat q)
{
    return rotate(glm::vec3(0, 1, 0), q);
}

inline glm::vec3 down(glm::quat q)
{
    return rotate(glm::vec3(0, -1, 0), q);
}

inline glm::vec3 right(glm::quat q)
{
    return rotate(glm::vec3(1, 0, 0), q);
}

inline glm::vec3 left(glm::quat q)
{
    return rotate(glm::vec3(-1, 0, 0), q);
}

inline glm::vec3 forward(glm::quat q)
{
    return glm::axis(q);
}

inline glm::vec3 backward(glm::quat q)
{
    return -glm::axis(q);
}

inline glm::vec3 to_vec3(glm::quat q)
{
    glm::vec3 v;

    v.x = 2 * (q.x * q.z - q.w * q.y);
    v.y = 2 * (q.y * q.z + q.w * q.x);
    v.z = 1 - 2 * (q.x * q.x + q.y * q.y);

    return v;
}

template<typename T>
inline T clamp(T x, T down, T up)
{
    if (x < down)
        return down;
    if (x > up)
        return up;
    return x;
}

template<typename T>
inline T saturate(T x)
{
    if (x > 1.0)
    {
        return 1.0;
    }

    if (x < 0.0)
    {
        return 0.0;
    }

    return x;
}
