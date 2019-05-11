// -*- C++ -*-
// sphere.hpp --
//

#pragma once

#include "aabb.hpp"


namespace lucid
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
	constexpr Intersection
	intersect(const Ray_<RayContainer>& ray,
              const Sphere_<SphereContainer>& prim) noexcept
	{
		const auto& [o, d] = ray;
		const auto a = d.dot(d);
		const auto pc = o - prim.center;
		const auto b = d.dot(pc * 2_r);
		const auto c = pc.dot(pc) - pow<2>(prim.radius);
        const auto [hit, t] = quadratic(a, b, c);
        return Intersection{hit, t, Vec2{}};
	}

	template <template <typename, size_t> typename SphereContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr Normal
    normal(const Ray_<RayContainer>& ray,
           const Intersection_<IsectContainer>& isect,
           const Sphere_<SphereContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto pos = o + d * isect.distance();
        return Normal(pos - prim.center);
    }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr Point
    sample(const Vec2_<SContainer>& s,
           const Sphere_<PContainer>& prim) noexcept
    {
        const auto& [c, r] = prim;
        const auto& [t1, t2] = s;
        const auto u = 2_r * t1 - 1;
        const auto theta = 2_r * math::PI<real> * t2;
        const auto _u = math::sqrt(1_r - pow<2>(u));
        return Point(_u * math::cos(theta),
                     _u * math::sin(theta),
                     u) * r + c;
    }

    template <template <typename, size_t> typename Container>
    constexpr Point
    centroid(const Sphere_<Container>& prim) noexcept
    { return prim.center; }

    template <template <typename, size_t> typename Container>
    constexpr AABB
    bound(const Sphere_<Container>& prim) noexcept
    {
        const auto& [c, r] = prim;
        return AABB{c - r, c + r};
    }
}
