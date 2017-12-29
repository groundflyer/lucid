// -*- C++ -*-
// intersection.hpp --
//

#pragma once

#include "ray.hpp"
#include "primitive.hpp"
#include <limits>


namespace yapt
{
    class Primitive;

    template <template <typename, size_t> class Container>
    class Intersection
    {
		bool m_intersect = false;
		real m_t = std::numeric_limits<real>::infinity(); // distance
		Vec3<Container> m_pos;
		Vec2<Container> m_st; 		// parametric coordinates
		Primitive const * hitprim = nullptr;

    public:
		constexpr
		Intersection() {}

		explicit constexpr
		Intersection(const bool i) : m_intersect(i) {}

		template <template <typename, size_t> class Container1>
		constexpr
		Intersection(const bool i,
					 const real & t,
					 const Ray<Container1> & ray) : m_intersect(i), m_t(t), m_pos(ray.origin()+ray.dir()*t) {}

		template <
			template <typename, size_t> class Container1,
			template <typename, size_t> class Container2>
		constexpr
		Intersection(const bool i,
					 const real & t,
					 const Ray<Container1> & ray,
					 const Primitive<Container2> * prim)
	    : m_intersect(i), m_t(t), m_pos(ray.origin()+ray.dir()*t), hitprim(prim) {}

		template <
			template <typename, size_t> class Container1,
			template <typename, size_t> class Container2,
			template <typename, size_t> class Container3>
		constexpr
		Intersection(const bool i,
					 const real & t,
					 const Ray<Container1> & ray,
					 const Primitive<Container2> * prim,
					 const Vec2<Container3> & st)
	    : m_intersect(i), m_t(t), m_pos(ray.origin()+ray.dir()*t), m_st(st), hitprim(prim) {}

		constexpr
		operator bool() const noexcept
		{ return m_intersect; }

		constexpr const Vec2<Container>&
		st() const noexcept
		{ return m_st; }

		constexpr real
		t() const noexcept
		{ return m_t; }

		constexpr const Vec3<Container>&
		pos() const noexcept
		{ return m_pos; }

		constexpr const Primitive &
		prim() const noexcept
		{ return *hitprim; }
    };
}
