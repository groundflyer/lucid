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
              const Triangle_<TriangleContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [p0, p1, p2] = prim;
        const auto edge1 = p1 - p0;
        const auto edge2 = p2 - p0;
        const auto pvec = d.cross(edge2);
        const auto D = edge1.dot(pvec);

        if (D > 0)
        {
            const auto tvec = o - p0;
            const auto u = tvec.dot(pvec);

            if (Range(0_r, D).encloses(u))
            {
                const auto qvec = tvec.cross(edge1);
                const auto v = d.dot(qvec);

                if (v >= 0 && (u + v) <= D)
                {
                    const auto invD = 1_r / D;
                    const auto t = edge2.dot(qvec) * invD;
                    return Intersection(range.encloses(t), t, Vec2(u, v) * invD);
                }
            }
        }

        return Intersection();
    }

	template <template <typename, size_t> typename TriangleContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    compute_normal(const Ray_<RayContainer>&,
                   const Intersection_<IsectContainer>&,
                   const Triangle_<TriangleContainer>& prim) noexcept
    {
        const auto& [p0, p1, p2] = prim;
        const auto edge1 = p1 - p0;
        const auto edge2 = p2 - p0;
        return Normal(edge1.cross(edge2));
    }
}
