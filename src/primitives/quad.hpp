// -*- C++ -*-
// quad.hpp
//

/// @file
/// Difinition of quadrilateral primitive.

#pragma once

#include "triangle.hpp"
#include <utils/range.hpp>

namespace lucid
{
/// @brief We define quadrilateral as an array of vertices.
///
/// Vertices set in this order:
/// v00, v01, v11, v10.
template <template <typename, size_t> typename Container>
using Quad_ = std::array<Vec3_<Container>, 4>;

using Quad = Quad_<std::array>;

namespace prim_fn
{
/// @brief Compute ray-quadrilateral intersection.
///
/// Implements @cite LD2004AERIT
template <template <typename, size_t> typename QuadContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Quad_<QuadContainer>& prim) noexcept
{
    const auto& [o, d]               = ray;
    const auto& [v00, v01, v11, v10] = prim;
    const Vec3 e01                   = v10 - v00;
    const Vec3 e03                   = v01 - v00;
    const Vec3 p                     = cross(d, e03);
    const real D                     = dot(e01, p);

    if(math::abs(D) < std::numeric_limits<real>::min()) return Intersection();

    const Vec3 T     = o - v00;
    const real alpha = dot(T, p) / D;

    const constexpr auto range01 = range(0_r, 1_r);

    if(!range01(alpha)) return Intersection();

    const Vec3 Q    = cross(T, e01);
    const real beta = dot(d, Q) / D;
    if(!range01(beta)) return Intersection();

    if((alpha + beta) > 1)
    {
        const Vec3 e23 = v01 - v11;
        const Vec3 e21 = v10 - v11;
        const Vec3 p_  = cross(d, e21);
        const real D_  = dot(e23, p_);

        if(math::abs(D_) < std::numeric_limits<real>::min()) return Intersection();

        const Vec3 T_     = o - v11;
        const real alpha_ = dot(T_, p_) / D_;

        if(alpha_ < 0) return Intersection();

        const Vec3 Q_    = cross(T_, e23);
        const real beta_ = dot(d, Q_) / D_;
        if(beta_ < 0) return Intersection();
    }

    const real t = dot(e03, Q) / D;

    if(t < 0) return Intersection();

    const Vec3 e02              = v11 - v00;
    const Vec3 N                = cross(e01, e03);
    const Vec3 aN               = abs(N);
    const auto& [Nx, Ny, Nz]    = N;
    const auto& [aNx, aNy, aNz] = aN;

    real a11{}, b11{};
    if((aNx >= aNy) && (aNx >= aNz))
    {
        a11 = (e02[1] * e03[2] - e02[2] * e03[1]) / Nx;
        b11 = (e01[1] * e02[2] - e01[2] * e02[1]) / Nx;
    }
    else if((aNy >= aNx) && (aNy >= aNz))
    {
        a11 = (e02[2] * e03[0] - e02[0] * e03[2]) / Ny;
        b11 = (e01[2] * e02[0] - e01[0] * e02[2]) / Ny;
    }
    else
    {
        a11 = (e02[0] * e03[1] - e02[1] * e03[0]) / Nz;
        b11 = (e01[0] * e02[1] - e01[1] * e02[0]) / Nz;
    }

    real u{}, v{};
    if(math::abs(a11 - 1) < std::numeric_limits<real>::min())
    {
        u = alpha;
        if(math::abs(b11 - 1) < std::numeric_limits<real>::min())
            v = beta;
        else
            v = beta / (u * (b11 - 1) + 1);
    }
    else if(math::abs(b11 - 1) < std::numeric_limits<real>::min())
    {
        v = beta;
        u = alpha / (v * (a11 - 1) + 1);
    }
    else
    {
        const real A     = -(b11 - 1);
        const real B     = alpha * (b11 - 1) - beta * (a11 - 1) - 1;
        const real C     = alpha;
        const real Delta = B * B - 4 * A * C;
        const real QQ    = -0.5_r * (B + std::copysign(math::sqrt(Delta), B));
        u                = QQ / A;

        if(!range01(u)) u = C / QQ;

        v = beta / (u * (b11 - 1) + 1);
    }

    return Intersection{true, t, Vec2(u, v)};
}

/// @brief Compute normal of a quadrilateral.
template <template <typename, size_t> typename QuadContainer,
          template <typename, size_t>
          typename PosContainer>
constexpr Vec3
normal(const Vec3_<PosContainer>&, const Quad_<QuadContainer>& prim) noexcept
{
    const auto e01 = std::get<3>(prim) - std::get<0>(prim);
    const auto e03 = std::get<1>(prim) - std::get<0>(prim);
    return normalize(cross(e03, e01));
}

/// @brief Sample a point on a quadrilateral surface.
template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Vec3
sample(const Vec2_<SContainer>& s, const Quad_<PContainer>& prim) noexcept
{
    const auto& [a, c1, b, c2] = prim;
    const auto& [t1, t2]       = s;
    const auto& c              = t1 > 0.5_r ? c1 : c2;
    return Vec3(detail::triangle_sample(Vec2(resample(t1), t2), a, b, c));
}

/// @brief Compute centroid of a quadrilateral.
template <template <typename, size_t> typename Container>
constexpr Vec3
centroid(const Quad_<Container>& prim) noexcept
{
    return centroid(detail::bound(prim));
}

/// @brief Compute bounding box of quadrilateral.
template <template <typename, size_t> typename Container>
constexpr AABB
bound(const Quad_<Container>& prim) noexcept
{
    return detail::bound(prim);
}

/// @brief Transform a quadrilateral using a transformation matrix.
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename QuadContainer>
constexpr auto
apply_transform(const Mat4_<MatContainer>& t, const Quad_<QuadContainer>& prim) noexcept
{
    return std::apply([&](const auto&... points) { return Quad{apply_transform_p(t, points)...}; },
                      prim);
}
} // namespace prim_fn
} // namespace lucid
