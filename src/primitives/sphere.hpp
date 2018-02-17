// -*- C++ -*-
// sphere.hpp --
//

#pragma once

#include <core/intersection.hpp>
#include <core/range.hpp>
#include <core/ray.hpp>


namespace yapt
{
    template <template <typename, size_t> typename PointContainer,
			  template <typename, size_t> typename NormalContainer>
    struct Sphere_
    {
		const Point_<PointContainer> center;
		const Normal_<NormalContainer> pole;
		const Normal_<NormalContainer> meridian;
		const real radius;

		Sphere_() = delete;

		template <template <typename, size_t> typename Container1,
				  template <typename, size_t> typename Container2 = std::array,
				  template <typename, size_t> typename Container3 = std::array>
		constexpr
		Sphere_(const Point_<Container1>& _center,
				const real& _radius,
				const Normal_<Container2>& _pole = Normal(0,1,0),
				const Normal_<Container3>& _meridian = Normal(0,0,1)) :
		center(_center),
			pole(_pole),
			meridian(_meridian),
			radius(_radius)
		{}

		// template <template <typename, size_t> typename Container1>
		// constexpr auto
		// normal(const Vec2_<Container1>& st) const noexcept
		// {
		// 	// const Normal_<Container> n(isect - center);
		// 	// return std::pair(n, Normal_<Container>(n.cross(n.cross(pole))));
		// }
	};

	template <template <typename, size_t> typename Container>
	Sphere_(const Point_<Container>&,
			const real&,
			const Normal_<Container>&,
			const Normal_<Container>&) -> Sphere_<Container, Container>;

	using Sphere = Sphere_<std::array, std::array>;

	template <template <typename, size_t> typename SpherePContainer,
			  template <typename, size_t> typename SphereNContainer,
			  template <typename, size_t> typename RayPContainer,
			  template <typename, size_t> typename RayNContainer>
	constexpr auto
	intersect(const Ray_<RayPContainer, RayNContainer>& ray,
              const Sphere_<SpherePContainer, SphereNContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
	{
		const auto [o, d] = ray;
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
