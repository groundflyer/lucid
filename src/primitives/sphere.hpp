// -*- C++ -*-
// sphere.hpp --
//

#pragma once

#include <core/intersection.hpp>
#include <core/range.hpp>
#include <core/ray.hpp>


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
			  template <typename, size_t> typename RayPContainer,
			  template <typename, size_t> typename RayNContainer>
	constexpr auto
	intersect(const Ray_<RayPContainer, RayNContainer>& ray,
              const Sphere_<SphereContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
	{
		const auto& [o, d] = ray;
		const auto a = d.dot(d);
		const auto pc = o - prim.center;
		const auto b = d.dot(pc * 2);
		const auto c = pc.dot(pc) - prim.radius * prim.radius;
		const auto D = b * b - 4 * a * c;

		if(D >= 0)
		{
			const auto factor = 0.5_r * a;

			if(D > 0)
			{
				const auto sqrtD = math::sqrt(D);
				const auto t1 = (-b + sqrtD) * factor;
				const auto t2 = (-b - sqrtD) * factor;

				if(t1 > 0)
				{
					if(t2 > 0)
					{
						decltype(auto) t = std::min(t1, t2);
						return Intersection(range.encloses(t), t, Vec2(0));
					}
					else
						return Intersection(range.encloses(t1), t1, Vec2(0));
				}
				else
				{
					const auto t = -b * factor;
					return Intersection(range.encloses(t), t, Vec2(0));
				}
			}
		}

        return Intersection();
	}
}
