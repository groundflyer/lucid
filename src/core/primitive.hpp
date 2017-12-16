// -*- C++ -*-
// primitive.hpp --
//

#pragma once

#include "ray.hpp"
#include "intersection.hpp"


namespace yapt
{
    template <template <typename, size_t>> class Container>
    class Intersection;

    class Primitive
    {
    public:
	// Primitive() {}

	template <template <typename, size_t> class Container>
	virtual	Intersection
	intersect(const Ray<Container> &,
		  const Range<real> &) const noexcept = 0;

	template <template <typename, size_t> class Container>
	virtual Vec3<Container>
	normal(const Intersection &) const noexcept = 0;

	template <template <typename, size_t> class Container>
	virtual Vec3<Container>
	tangent(const Intersection &) const noexcept = 0;
    };

}
