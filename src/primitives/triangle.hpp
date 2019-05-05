// -*- C++ -*-
// triangle.h --
//

#pragma once

#include "aabb.hpp"


namespace lucid
{
    template <template <typename, size_t> typename Container>
    using Triangle_ = std::array<Point_<Container>, 3>;

    using Triangle = Triangle_<std::array>;


	template <template <typename, size_t> typename TriangleContainer,
			  template <typename, size_t> typename RayContainer>
	constexpr Intersection
	intersect(const Ray_<RayContainer>& ray,
              const Triangle_<TriangleContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [v0, v1, v2] = prim;
        const auto edge1 = v1 - v0;
        const auto edge2 = v2 - v0;
        const auto pvec = d.cross(edge2);
        const auto D = edge1.dot(pvec);
        const auto tvec = o - v0;
        const auto u = tvec.dot(pvec);
        const auto qvec = tvec.cross(edge1);
        const auto v = d.dot(qvec);
        const auto invD = 1_r / D;
        const auto t = edge2.dot(qvec) * invD;
        return Intersection{D > 0_r && u < D && v >= 0_r && (u + v) <= D, t, Vec2{u, v} * invD};
    }

	template <template <typename, size_t> typename TriangleContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr Normal
    normal(const Ray_<RayContainer>&,
           const Intersection_<IsectContainer>&,
           const Triangle_<TriangleContainer>& prim) noexcept
    {
        const auto& [v0, v1, v2] = prim;
        const auto edge1 = v1 - v0;
        const auto edge2 = v2 - v0;
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

        // R. Osada et al. Shape distributions. 2002
        // Eric Heitz A Low-Distortion Map Between Triangle and Square
        template <template <typename, size_t> typename Container>
        constexpr auto
        triangle_sample(const Vec2_<Container>& s,
                        const Point_<Container>& a,
                        const Point_<Container>& b,
                        const Point_<Container>& c) noexcept
        {
            auto [t1, t2] = s;
            if (t1 > t2)
                shift(t1, t2);
            else
                shift(t2, t1);

            return a * t1 + b * t2 + c * (1_r - t1 - t2);
        }

        template <typename Prim>
        constexpr auto
        bound(const Prim& prim) noexcept
        {
            const Point vmin(std::apply([](const auto& v1, const auto& ... verts)
                                        {
                                            return reduce([](const auto& a, const auto& b){ return lucid::min(a, b);},
                                                          Vec3(v1),
                                                          Vec3(verts)...);
                                        },
                    prim));

            const Point vmax(std::apply([](const auto& v1, const auto& ... verts)
                                        {
                                            return reduce([](const auto& a, const auto& b){ return lucid::max(a, b);},
                                                          Vec3(v1),
                                                          Vec3(verts)...);
                                        },
                    prim));
            return AABB{vmin, vmax};
        }
    }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr Point
    sample(const Vec2_<SContainer>& s,
           const Triangle_<PContainer>& prim) noexcept
    { return Point(std::apply([&](const auto& ... verts){ return detail::triangle_sample(s, verts...); }, prim)); }

    template <template <typename, size_t> typename Container>
    constexpr Point
    centroid(const Triangle_<Container>& prim) noexcept
    { return centroid(detail::bound(prim)); }

    template <template <typename, size_t> typename Container>
    constexpr AABB
    bound(const Triangle_<Container>& prim) noexcept
    { return detail::bound(prim); }
}
