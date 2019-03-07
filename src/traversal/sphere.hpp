// -*- C++ -*-
// sphere.hpp --
//

#pragma once

#include "intersection.hpp"
#include "ray.hpp"
#include <base/range.hpp>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Sphere_
    {
		Point_<Container> center;
		real radius;

        constexpr
		Sphere_() {}

		template <template <typename, size_t> typename Container1>
		constexpr
		Sphere_(const Point_<Container1>& _center,
				const real& _radius) :
		center(_center),
			radius(_radius)
		{}
	};

	template <template <typename, size_t> typename Container>
	Sphere_(const Point_<Container>&,
			const real&) -> Sphere_<Container>;

	using Sphere = Sphere_<std::array>;

	template <template <typename, size_t> typename SphereContainer,
			  template <typename, size_t> typename RayContainer>
	constexpr auto
	intersect(const Ray_<RayContainer>& ray,
              const Sphere_<SphereContainer>& prim) noexcept
	{
		const auto& [o, d] = ray;
		const auto a = d.dot(d);
		const auto pc = o - prim.center;
		const auto b = d.dot(pc * 2_r);
		const auto c = pc.dot(pc) - math::pow<2>(prim.radius);
		const auto D = b * b - 4_r * a * c;
        const auto factor = 0.5_r * a;
        const auto sqrtD = math::sqrt(D);
        const auto t1 = (-b + sqrtD) * factor;
        const auto t2 = (-b - sqrtD) * factor;
        const auto t = t1 > 0_r ? (t2 > 0_r ? std::min(t1, t2) : t1) : -b * factor;
        return Intersection{D > 0, t, Vec2{}};
	}

	template <template <typename, size_t> typename SphereContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    compute_normal(const Ray_<RayContainer>& ray,
                   const Intersection_<IsectContainer>& isect,
                   const Sphere_<SphereContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto pos = o + d * isect.distance();
        return Normal(pos - prim.center);
    }
}
