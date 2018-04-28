// -*- C++ -*-
// bbox.hpp --
//

#pragma once

#include <core/intersection.hpp>
#include <core/range.hpp>
#include <core/ray.hpp>


namespace yapt
{
    // axis-aligned bounding box
    template <template <typename, size_t> typename Container>
    struct AABB_
    {
		std::array<Point_<Container>, 2> bounds;

        constexpr
		AABB_() {};

		template <template <typename, size_t> typename Container1,
				  template <typename, size_t> typename Container2>
		constexpr
		AABB_(const Point_<Container1>& vmin,
			  const Point_<Container2>& vmax) :
		bounds({vmin, vmax}) {}
	};

	template <template <typename, size_t> typename Container>
	AABB_(const Point_<Container>&,
		  const Point_<Container>&) -> AABB_<Container>;

	using AABB = AABB_<std::array>;


	template <template <typename, size_t> typename AABBContainer,
			  template <typename, size_t> typename RayPContainer,
			  template <typename, size_t> typename RayNContainer>
	constexpr auto
	intersect(const Ray_<RayPContainer, RayNContainer>& ray,
              const AABB_<AABBContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
	{
		const auto& bounds = prim.bounds;
		const auto& [o, d] = ray;
		const auto& [ox, oy, oz] = o;
		const auto inv_d = Vec3(1) / d;
		const auto [signx, signy, signz] = inv_d < 0;
		const auto& [ix, iy, iz] = inv_d;

		auto xmin = ix * (bounds[signx][0] - ox);
		auto xmax = ix * (bounds[!signx][0] - ox);

		auto ymin = iy * (bounds[signy][1] - oy);
		auto ymax = iy * (bounds[!signy][1] - oy);

		if ((xmin > ymax) || (ymin > xmax))
			return Intersection();

		if (ymin > xmin) xmin = ymin;
		if (ymax < xmax) xmax = ymax;

		auto zmin = iz * (bounds[signz][2] - oz);
		auto zmax = iz * (bounds[!signz][2] - oz);

		if ((xmin > zmax) || (zmin > xmax))
			return Intersection();

		if (zmin > xmin) xmin = zmin;

		return Intersection(range.encloses(xmin), xmin, Vec2());
	}
}
