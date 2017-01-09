// -*- C++ -*-
// sphere.cpp -- 

#include "sphere.hpp"


namespace yapt
{
    Sphere::Sphere() {}

    Sphere::Sphere(const Vec3 & p, const real r) : _p(p), _r(r) {}

    Intersection
    Sphere::intersect(const Ray & ray,
		      const real & t_min, const real & t_max) const noexcept
    {
	ASSERT(t_min >= 0 || t_max > 0 || t_min < t_max);
	// quadratic coefficients
	const auto a = ray.dir() % ray.dir();
	const auto pc = ray.origin() - _p;
	const auto b = ray.dir() % pc * 2;
	const auto c = pc % pc - _r*_r;

	const auto D = b*b - 4 * a * c;

	// roots
	real t = 0, t1 = 0, t2 = 0;
	bool intersected = false;

	if (D >= 0)
	    {
		const auto factor = 1 / (2 * a);
		if (D > 0)
		    {
			const auto sqrtD = std::sqrt(D);
			t1 = (-b + sqrtD) * factor;
			t2 = (-b - sqrtD) * factor;

			if (t1 > 0)
			    if (t2 > 0)
				t = std::min(t1, t2);
			    else
				t = t1;
			else
			    t = t2;
		    }
		else
		    t = -b * factor;

		intersected = in_range(t, t_min, t_max);
	    }

	return Intersection(intersected, t, ray, this);
    }

    Vec3
    Sphere::normal(const Intersection & isect) const noexcept
    {
	return normalize(isect.pos() - _p);
    }

    Vec3
    Sphere::tangent(const Intersection & isect) const noexcept
    {
	Vec3 n = normal(isect);
	Vec3 tmp1 = normalize(n ^ Vec3(0, 1, 0));
	return normalize(n ^ tmp1);
    }
}
