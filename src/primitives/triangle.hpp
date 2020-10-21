// -*- C++ -*-
// triangle.h --
//

/// @file
/// Triangle primitive definitions.

#pragma once

#include "aabb.hpp"
#include <utils/range.hpp>

namespace lucid
{
/// @brief Defining triangle as array of vertices.
template <template <typename, size_t> typename Container>
using Triangle_ = std::array<Vec3_<Container>, 3>;

using Triangle = Triangle_<std::array>;

/// @brief Compute ray-triangle intersection.
///
/// Implements classic algorithm described in
/// @cite 10.1080/10867651.1997.10487468
template <template <typename, size_t> typename TriangleContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Triangle_<TriangleContainer>& prim) noexcept
{
    const auto& [o, d]       = ray;
    const auto& [v0, v1, v2] = prim;
    const Vec3 edge1         = v1 - v0;
    const Vec3 edge2         = v2 - v0;
    const Vec3 pvec          = cross(d, edge2);
    const real D             = dot(edge1, pvec);
    const real invD          = 1_r / D;
    const Vec3 tvec          = o - v0;
    const real u             = dot(tvec, pvec) * invD;
    const Vec3 qvec          = cross(tvec, edge1);
    const real v             = dot(d, qvec) * invD;
    const real t             = dot(edge2, qvec) * invD;
    const bool not_intersected =
        almost_equal(D, 0_r, 10) || std::signbit(v) || std::signbit(t) || u + v > 1_r;

    const bool intersected = range(0_r, 1_r)(u) && !not_intersected;

    return Intersection{intersected, t, Vec2{u, v}};
}

/// @brief Compute triangle normal.
template <template <typename, size_t> typename TriangleContainer,
          template <typename, size_t>
          typename PosContainer>
constexpr Vec3
normal(const Vec3_<PosContainer>&, const Triangle_<TriangleContainer>& prim) noexcept
{
    const auto& [v0, v1, v2] = prim;
    const Vec3 edge1         = v1 - v0;
    const Vec3 edge2         = v2 - v0;
    return normalize(cross(edge1, edge2));
}

namespace detail
{
constexpr void
shift(real& x, real& y) noexcept
{
    x *= 0.5_r;
    y -= x;
}

constexpr Vec2&
s2t(Vec2& s) noexcept
{
    auto& [x, y] = s;

    if(y > x)
        shift(x, y);
    else
        shift(y, x);

    return s;
}

template <template <typename, size_t> typename Container2>
constexpr Vec3
triangle_sample(Vec2                     s,
                const Vec3_<Container2>& v1,
                const Vec3_<Container2>& v2,
                const Vec3_<Container2>& v3) noexcept
{
    auto& [t1, t2] = s2t(s);
    return v1 * t1 + v2 * t2 + v3 * (1_r - t1 - t2);
}

/// @brief Compute bounding box of primitive defined as array.
template <typename Prim>
constexpr AABB
bound(const Prim& prim) noexcept
{
    const Vec3 vmin(std::apply(
        [](const auto& v1, const auto&... verts) {
            return reduce(
                static_cast<Vec3 (*)(const Vec3&, const Vec3&)>(lucid::min), v1, verts...);
        },
        prim));

    const Vec3 vmax(std::apply(
        [](const auto& v1, const auto&... verts) {
            return reduce(
                static_cast<Vec3 (*)(const Vec3&, const Vec3&)>(lucid::max), v1, verts...);
        },
        prim));
    return AABB{vmin, vmax};
}
} // namespace detail

/// @brief Sample a point on triangle surface.
///
/// Optimized implementation from @cite heitz:hal-02073696
template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Vec3
sample(const Vec2_<SContainer>& s, const Triangle_<PContainer>& prim) noexcept
{
    return Vec3(std::apply(
        [&](const auto&... verts) { return detail::triangle_sample(s, verts...); }, prim));
}

/// @brief Compute triangle centroid.
template <template <typename, size_t> typename Container>
constexpr Vec3
centroid(const Triangle_<Container>& prim) noexcept
{
    return centroid(detail::bound(prim));
}

/// @brief Compute triangle bounding box.
template <template <typename, size_t> typename Container>
constexpr AABB
bound(const Triangle_<Container>& prim) noexcept
{
    return detail::bound(prim);
}

/// @brief Transform triangle with a transformation matrix.
template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename TriangleContainer>
constexpr auto
apply_transform(const Mat4_<MatContainer>& t, const Triangle_<TriangleContainer>& prim) noexcept
{
    return std::apply(
        [&](const auto&... points) { return Triangle{apply_transform_p(t, points)...}; }, prim);
}
} // namespace lucid
