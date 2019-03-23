// -*- C++ -*-
// triangle.h --
//

#pragma once

#include "intersection.hpp"
#include "ray.hpp"
#include <base/range.hpp>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Triangle_
    {
        Point_<Container> p0;
        Point_<Container> p1;
        Point_<Container> p2;

        constexpr
        Triangle_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2,
                  template <typename, size_t> typename Container3>
        constexpr
        Triangle_(const Point_<Container1>& _p0,
                  const Point_<Container2>& _p1,
                  const Point_<Container3>& _p2) :
        p0(_p0), p1(_p1), p2(_p2)
        {}
    };


    template <template <typename, size_t> typename Container>
    Triangle_(const Point_<Container>&,
              const Point_<Container>&,
              const Point_<Container>&) -> Triangle_<Container>;

    using Triangle = Triangle_<std::array>;


	template <template <typename, size_t> typename TriangleContainer,
			  template <typename, size_t> typename RayContainer>
	constexpr auto
	intersect(const Ray_<RayContainer>& ray,
              const Triangle_<TriangleContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [p0, p1, p2] = prim;
        const auto edge1 = p1 - p0;
        const auto edge2 = p2 - p0;
        const auto pvec = d.cross(edge2);
        const auto D = edge1.dot(pvec);
        const auto tvec = o - p0;
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
    constexpr auto
    normal(const Ray_<RayContainer>&,
           const Intersection_<IsectContainer>&,
           const Triangle_<TriangleContainer>& prim) noexcept
    {
        const auto& [p0, p1, p2] = prim;
        const auto edge1 = p1 - p0;
        const auto edge2 = p2 - p0;
        return Normal(edge1.cross(edge2));
    }

    // R. Osada et al. Shape distributions. 2002
    template <template <typename, size_t> typename Container>
    constexpr auto
    triangle_sample(const real r1,
                    const real r2,
                    const Point_<Container>& a,
                    const Point_<Container>& b,
                    const Point_<Container>& c) noexcept
    {
        const auto sqr1 = math::sqrt(r1);
        return a * (1_r - sqr1) + b * sqr1 * (1_r - r2) + c * sqr1 * r2;
    }

	template <template <typename, size_t> typename Container,
              typename Generator>
    constexpr auto
    sample(Generator&& gen,
           const Triangle_<Container>& prim) noexcept
    {
        const auto& [p0, p1, p2] = prim;
        return triangle_sample(gen(), gen(), p0, p1, p2);
    }
}
