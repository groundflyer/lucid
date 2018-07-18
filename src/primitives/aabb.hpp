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
        Point_<Container> m_vmin;
        Point_<Container> m_vmax;

    public:
        constexpr
		AABB_() {};

		template <template <typename, size_t> typename Container1,
				  template <typename, size_t> typename Container2>
		constexpr
		AABB_(const Point_<Container1>& vmin,
			  const Point_<Container2>& vmax) : m_vmin(vmin), m_vmax(vmax) {}

        constexpr const auto&
        operator[](const std::uint8_t i) const noexcept
        {
            CHECK_INDEX(i, 2);
            return i ? m_vmax : m_vmin;
        }

        constexpr auto&
        operator[](const std::uint8_t i) noexcept
        {
            CHECK_INDEX(i, 2);
            return i ? m_vmax : m_vmin;
        }

        template <template <typename, size_t> typename Container1>
        constexpr auto
        operator[](const Vector<bool, 3, Container1>& idxs) const noexcept
        {
            Vec3 ret;
            for(std::uint8_t i = 0; i < 3; ++i)
                ret[i] = (*this)[idxs[i]][i];
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

        const auto mm = max(vmin);
        return Intersection(range.encloses(mm), mm, Vec2());
	}

	template <template <typename, size_t> typename AABBContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    compute_normal(const Ray_<RayContainer>& ray,
                   const Intersection_<IsectContainer>& isect,
                   const AABB_<AABBContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto pos = o + d * isect.distance();
        const real nsign[2] {-1_r, 1_r};
        const auto dd0 = pos - prim[0];
        const auto dd1 = pos - prim[1];
        const auto ad0 = abs(dd0);
        const auto ad1 = abs(dd1);
        const auto md0 = min(ad0);
        const auto md1 = min(ad1);
        const auto vd0 = Vec3(ad0 == md0);
        const auto vd1 = Vec3(ad1 == md1);
        const auto pp = md0 > md1;
        const auto& dd = pp ? vd1 : vd0;
        return Normal(dd * nsign[pp]);
    }
}
