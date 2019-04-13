// -*- C++ -*-
// triangle.h --
//

#pragma once

#include <base/intersection.hpp>
#include <base/ray.hpp>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Triangle_
    {
        Point_<Container> v0;
        Point_<Container> v1;
        Point_<Container> v2;

        constexpr
        Triangle_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2,
                  template <typename, size_t> typename Container3>
        constexpr
        Triangle_(const Point_<Container1>& _v0,
                  const Point_<Container2>& _v1,
                  const Point_<Container3>& _v2) :
        v0(_v0), v1(_v1), v2(_v2)
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
    constexpr auto
    normal(const Ray_<RayContainer>&,
           const Intersection_<IsectContainer>&,
           const Triangle_<TriangleContainer>& prim) noexcept
    {
        const auto& [v0, v1, v2] = prim;
        const auto edge1 = v1 - v0;
        const auto edge2 = v2 - v0;
        return Normal(edge1.cross(edge2));
    }

    namespace impl
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
    }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr auto
    sample(const Vec2_<SContainer>& s,
           const Triangle_<PContainer>& prim) noexcept
    {
        const auto& [v0, v1, v2] = prim;
        return Point(impl::triangle_sample(s, v0, v1, v2));
    }

    template <template <typename, size_t> typename Container>
    constexpr auto
    centroid(const Triangle_<Container>& prim) noexcept
    {
        const auto& [v0, v1, v2] = prim;
        return Point((v0 + v1 + v2) / 3_r);
    }
}
