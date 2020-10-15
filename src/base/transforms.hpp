// -*- C++ -*-
// transforms.hpp --
//

/// @file
/// Defines coordinate transformation functions.

#pragma once

#include "types.hpp"

#include <utility>

namespace lucid
{
/// @brief Build a normalized vector.
template <typename... Args>
constexpr Vec3
make_normal(const Args&... args) noexcept
{
    return normalize(Vec3(args...));
}

/// @brief Convert a vector from homogenous coordinates.
/// @param a homogenous column vector.
template <template <typename, size_t> typename Container>
constexpr Vec3
dehomogenize(const Vec4_<Container>& a) noexcept
{
    Vec3       ret{a};
    const real w = get_w(a);
    if(!(almost_equal(w, 0_r, 5) || almost_equal(w, 1_r, 5))) ret /= w;
    return ret;
}

/// @brief Apply transform to a vector.
/// @param t transform to apply.
/// @param v vector to transform.
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename VecContainer>
constexpr Vec3
apply_transform(const Mat4_<MatContainer>& t, const Vec3_<VecContainer>& v) noexcept
{
    return dehomogenize(dot(t, Vec4(v)));
}

/// @brief Apply transform to a point.
/// @param t transform to apply.
/// @param p point to transform.
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename Vec3Container>
constexpr Vec3
apply_transform_p(const Mat4_<MatContainer>& t, const Vec3_<Vec3Container>& p) noexcept
{
    return dehomogenize(dot(t, Vec4(p, 1)));
}

/// @brief Apply transform to a normal.
/// @param t transform to apply.
/// @param n noraml to transform.
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename NormalContainer>
constexpr Vec3
apply_transform_n(const Mat4_<MatContainer>& t, const Vec3_<NormalContainer>& n) noexcept
{
    return normalize(dehomogenize(dot(transpose(inverse(t)), (Vec4(n)))));
}

/// @brief Create translation transform.
/// @param delta translation delta.
template <template <typename, size_t> typename Container>
constexpr Mat4
translate(const Vec3_<Container>& delta) noexcept
{
    const auto& [x, y, z] = delta;
    return Mat4(1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1);
}

/// @brief Create scale transform.
/// @param delta scale delta.
template <template <typename, size_t> typename Container>
constexpr Mat4
scale(const Vec3_<Container>& delta) noexcept
{
    const auto& [x, y, z] = delta;
    return Mat4(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
}

/// @brief Create clockwise rotation transform.
/// @param angle angle defined in radians.
/// @param axis axis to rotate object around.
template <template <typename, size_t> typename Container>
constexpr Mat4
rotate(const real angle, const Vec3_<Container>& axis) noexcept
{
    const auto& [x, y, z] = axis;

    const Mat3 K{0, -z, y, z, 0, -x, -y, x, 0};

    return Mat4(Mat3::identity() + K * math::sin(angle) + dot(K, K) * (1_r - math::cos(angle)));
}

/// @brief Create Look-At transform.
/// @param eye eye position.
/// @param target target position.
/// @param up vector (usually {0, 1, 0}).
template <template <typename, size_t> typename EyeContainer,
          template <typename, size_t>
          typename CenContainer,
          template <typename, size_t>
          typename UpContainer>
constexpr Mat4
look_at(const Vec3_<EyeContainer>& eye,
        const Vec3_<CenContainer>& target,
        const Vec3_<UpContainer>&  up) noexcept
{
    const Vec3 f = normalize(eye - target);
    const Vec3 r = normalize(cross(up, f));
    const Vec3 u = normalize(cross(f, r));
    return inverse(dot(Mat4(r, 0_r, u, 0_r, f, 0_r, 0_r, 0_r, 0_r, 1_r), (translate(-eye))));
}

/// @brief Build orthonormal basis for a given normal (z axis).
/// @cite Duff2017Basis
/// @return a pair of normals representing x and y axes.
template <template <typename, size_t> typename Container>
constexpr std::pair<Vec3, Vec3>
basis(const Vec3_<Container>& n) noexcept
{
    const auto& [nx, ny, nz] = n;
    const real sign          = std::copysign(1_r, nz);
    const real a             = -1_r / (sign + nz);
    const real b             = nx * ny * a;
    return std::pair(make_normal(1_r + sign * pow<2>(nx) * a, sign * b, -sign * nx),
                     make_normal(b, sign + pow<2>(ny) * a, -ny));
}

/// @brief Build orthonormal basis for a given normal (z axis).
/// @cite Duff2017Basis
/// @return 3x3 rotation matrix.
template <template <typename, size_t> typename Container>
constexpr Mat3
basis_matrix(const Vec3_<Container>& z) noexcept
{
    const auto [x, y] = basis(z);
    return transpose(Mat3(x, y, z));
}
} // namespace lucid
