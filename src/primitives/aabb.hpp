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
    class AABB_
    {
        Container<real, 6> m_data;

    public:
        constexpr
		AABB_() {};

        constexpr
        AABB_(Container<real, 6>&& cont) : m_data(cont) {}

		template <template <typename, size_t> typename Container1,
				  template <typename, size_t> typename Container2>
		constexpr
		AABB_(const Point_<Container1>& vmin,
			  const Point_<Container2>& vmax) :
        m_data({vmin[0], vmin[1], vmin[2], vmax[0], vmax[1], vmax[2]}) {}

        const constexpr auto
        operator[](const size_t i) const noexcept
        {
            CHECK_INDEX(i, 2);
            return Vector(ArrayView<real, 3>(const_cast<real*>(&m_data[i * 3])));
        }

        constexpr auto
        operator[](const size_t i) noexcept
        {
            CHECK_INDEX(i, 2);
            return Vector(ArrayView<real, 3>(&m_data[i * 3]));
        }

        template <template <typename, size_t> typename Container1>
        constexpr auto
        operator[](const Vector<bool, 3, Container1>& idxs) const noexcept
        {
            Vec3 ret;
            for(size_t i = 0; i < 3; ++i)
                ret[i] = m_data[idxs[i] * 3];
            return ret;
        }
	};

	template <template <typename, size_t> typename Container>
	AABB_(const Point_<Container>&,
		  const Point_<Container>&) -> AABB_<Container>;

	using AABB = AABB_<std::array>;


	template <template <typename, size_t> typename AABBContainer,
			  template <typename, size_t> typename RayContainer>
	constexpr auto
	intersect(const Ray_<RayContainer>& ray,
              const AABB_<AABBContainer>& prim,
			  const Range<real>& range = Range<real>()) noexcept
	{
		const auto& [o, d] = ray;
		const auto inv_d = Vec3(1) / d;
		const auto sign = inv_d < 0;
        const auto vmin = inv_d * (prim[sign] - o);
        const auto vmax = inv_d * (prim[!sign] - o);
        const auto& [xmin, ymin, zmin] = vmin;
        const auto& [xmax, ymax, zmax] = vmax;

        if(xmin > ymax ||
           ymin > xmax ||
           std::max(xmin, ymin) > zmax ||
           zmin > std::min(ymax, xmax))
            return Intersection();

        decltype(auto) mm = max(vmin);
        return Intersection(range.encloses(mm), mm, Vec2());
	}
}
