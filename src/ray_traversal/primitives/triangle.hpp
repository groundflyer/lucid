// -*- C++ -*-
// triangle.h --
//

#pragma once

#include "aabb.hpp"
#include <utils/range.hpp>

namespace lucid
{
template <template <typename, size_t> typename Container>
using Triangle_ = std::array<Point_<Container>, 3>;

using Triangle = Triangle_<std::array>;

template <template <typename, size_t> typename TriangleContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const Triangle_<TriangleContainer>& prim) noexcept
{
    const auto& [o, d]       = ray;
    const auto& [v0, v1, v2] = prim;
    const auto edge1         = v1 - v0;
    const auto edge2         = v2 - v0;
    const auto pvec          = d.cross(edge2);
    const auto D             = edge1.dot(pvec);
    const auto invD          = 1_r / D;
    const auto tvec          = o - v0;
    const auto u             = tvec.dot(pvec) * invD;
    const auto qvec          = tvec.cross(edge1);
    const auto v             = d.dot(qvec) * invD;
    const auto t             = edge2.dot(qvec) * invD;
    const bool not_intersected =
        almost_equal(D, 0_r, 10) || std::signbit(v) || std::signbit(t) || u + v > 1_r;

    const bool intersected = range(0_r, 1_r)(u) && !not_intersected;

    return Intersection{intersected, t, Vec2{u, v}};
}

template <template <typename, size_t> typename TriangleContainer,
          template <typename, size_t>
          typename RayContainer,
          template <typename, size_t>
          typename IsectContainer>
constexpr Normal
normal(const Ray_<RayContainer>&,
       const Intersection_<IsectContainer>&,
       const Triangle_<TriangleContainer>& prim) noexcept
{
    const auto& [v0, v1, v2] = prim;
    const auto edge1         = v1 - v0;
    const auto edge2         = v2 - v0;
    return Normal(edge1.cross(edge2));
}

namespace detail
{
constexpr void
shift(real& x, real& y) noexcept
{
    x *= 0.5_r;
    y -= x;
}

template <template <typename, size_t> typename Container>
constexpr auto
s2t(Vec2_<Container> s) noexcept
{
    auto& [x, y] = s;

    if(y > x)
        shift(x, y);
    else
        shift(y, x);

    return s;
}

// Eric Heitz A Low-Distortion Map Between Triangle and Square, 2019
template <template <typename, size_t> typename Container1,
          template <typename, size_t>
          typename Container2>
constexpr auto
triangle_sample(const Vec2_<Container1>&  s,
                const Point_<Container2>& v1,
                const Point_<Container2>& v2,
                const Point_<Container2>& v3) noexcept
{
    const auto [t1, t2] = s2t(s);
    return v1 * t1 + v2 * t2 + v3 * (1_r - t1 - t2);
}

template <typename Prim>
constexpr auto
bound(const Prim& prim) noexcept
{
    const Point vmin(std::apply(
        [](const auto& v1, const auto&... verts) {
            return reduce(
                [](const auto& a, const auto& b) { return lucid::min(a, b); }, v1, verts...);
        },
        prim));

    const Point vmax(std::apply(
        [](const auto& v1, const auto&... verts) {
            return reduce(
                [](const auto& a, const auto& b) { return lucid::max(a, b); }, v1, verts...);
        },
        prim));
    return AABB{vmin, vmax};
}
} // namespace detail

template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Point
sample(const Vec2_<SContainer>& s, const Triangle_<PContainer>& prim) noexcept
{
    return Point(std::apply(
        [&](const auto&... verts) { return detail::triangle_sample(s, verts...); }, prim));
}

template <template <typename, size_t> typename Container>
constexpr Point
centroid(const Triangle_<Container>& prim) noexcept
{
    return centroid(detail::bound(prim));
}

template <template <typename, size_t> typename Container>
constexpr AABB
bound(const Triangle_<Container>& prim) noexcept
{
    return detail::bound(prim);
}

template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename TriangleContainer>
constexpr auto
apply_transform(const Mat4_<MatContainer>& t, const Triangle_<TriangleContainer>& prim) noexcept
{
    return std::apply(
        [&](const auto&... points) { return Triangle{apply_transform(t, points)...}; }, prim);
}
} // namespace lucid
