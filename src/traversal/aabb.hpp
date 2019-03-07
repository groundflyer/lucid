// -*- C++ -*-
// bbox.hpp --
//

#pragma once

#include "intersection.hpp"
#include "ray.hpp"
#include <base/range.hpp>


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
        operator[](const size_t i) const noexcept
        {
            CHECK_INDEX(i, 2);
            return i ? m_vmax : m_vmin;
        }

        constexpr auto&
        operator[](const size_t i) noexcept
        {
            CHECK_INDEX(i, 2);
            return i ? m_vmax : m_vmin;
        }

        template <template <typename, size_t> typename Container1>
        constexpr auto
        operator[](const Vector<bool, 3, Container1>& idxs) const noexcept
        {
            Vec3 ret;
            for(size_t i = 0; i < 3; ++i)
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
              const AABB_<AABBContainer>& prim) noexcept
	{
		const auto& [o, d] = ray;
		const auto inv_d = Vec3{1_r} / d;
		const auto sign = inv_d < 0;
        const auto vmin = inv_d * (prim[sign] - o);
        const auto vmax = inv_d * (prim[!sign] - o);
        const auto& [xmin, ymin, zmin] = vmin;
        const auto& [xmax, ymax, zmax] = vmax;
        const auto intersected = xmin < ymax && ymin < xmax &&
            std::max(xmin, ymin) < zmax && zmin < std::min(ymax, xmax);
        return Intersection{intersected, max(vmin), Vec2{}};
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
        Vec3 vd[2] {};
        real md[2] {};
        for(unsigned i = 0; i < 2; ++i)
        {
            const auto ad = abs(pos - prim[i]);
            md[i] = min(ad);
            vd[i] = Vec3(ad == md[i]);
        }
        const auto pp = md[0] > md[1];
        return Normal(vd[pp] * nsign[pp]);
    }
}
