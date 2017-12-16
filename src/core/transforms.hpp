// -*- C++ -*-
// transforms.hpp --
//

#pragma once

#include "ray.hpp"

#include <utility>
#include <cmath>


namespace yapt
{
    template <template <typename, size_t> class Container>
    constexpr Vec4<Container>
    homogenize(const Point<Container> & a) noexcept
    {
	Vec4<Container> ret(a);
	ret[3] = 1;
	return ret;
    }

    template <template <typename, size_t> class Container>
    constexpr Vec3<Container>
    dehomogenize(const Vec4<Container> & a) noexcept
    { return Vec3<Container>(a) / (a[3] != 0 ? a[3] : 1); }


    template <
    	template <typename, size_t> class MatContainer,
    	template <typename, size_t> class VecContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
    		    const Vec3<VecContainer> & v) noexcept
    { return dehomogenize(t * Vec4_(v)); }


    template <
    	template <typename, size_t> class MatContainer,
    	template <typename, size_t> class PointContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
    		    const Point<PointContainer> & p) noexcept
    { return dehomogenize(t * homogenize(p)); }

    template <
    	template <typename, size_t> class MatContainer,
    	template <typename, size_t> class NormalContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
    		    const Normal<NormalContainer> & n) noexcept
    { return dehomogenize(transpose(inverse(t)) * Vec4_(n)); }

    template <
    	template <typename, size_t> class MatContainer,
    	template <typename, size_t> class RayContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
		    const Ray<RayContainer> & ray)
    { return Ray<RayContainer>(apply_transform(t, ray.origin()), apply_transform(t, ray.dir())); }


    template <template <typename, size_t> class Container>
    constexpr auto
    translate(const Vec3<Container> & delta) noexcept
    {
	return Mat4<Container>(1, 0, 0, delta[0],
			       0, 1, 0, delta[1],
			       0, 0, 1, delta[2],
			       0, 0, 0, 1);
    }

    template <template <typename, size_t> class Container>
    constexpr auto
    scale(const Vec3<Container> & delta) noexcept
    {
	return Mat4<Container>(delta[0], 0, 0, 0,
			       0, delta[1], 0, 0,
			       0, 0, delta[2], 0,
			       0, 0, 0, 1);
    }

    template <
	template <typename, size_t> class EyeContainer,
	template <typename, size_t> class CenContainer,
	template <typename, size_t> class UpContainer>
    constexpr auto
    look_at(const Point<EyeContainer> & eye,
	    const Point<CenContainer> & center,
	    const Normal<UpContainer> & up) noexcept
    {
	const Vec3_ f = normalize(center - eye);
	const auto s = normalize(f.cross(up));
	const auto u = s.cross(f);

	return Mat4_(s[0], s[1], s[2], 0,
				   u[0], u[1], u[2], 0,
				   -f[0], -f[1], -f[2], 0,
				   0, 0, 0, 1)
	    * translate(-eye);
    }


    template <template <typename, size_t> class Container>
    constexpr auto
    basis(const Vec3<Container> & v1) noexcept
    {
	Vec3_ v2(0);

	if (std::abs(v1[0]) > std::abs(v1[1]))
	{
	    const auto il = real(1) / (std::sqrt(v1[0]*v1[0] + v1[2]*v1[2]));
	    v2[0] = -v1[2] * il;
	    v2[2] = v1[0] * il;
	}
	else
	{
	    const auto il = real(1) / (std::sqrt(v1[1]*v1[1] + v1[2]*v1[2]));
	    v2[1] = v1[2] * il;
	    v2[2] = -v1[1] * il;
	}

	const auto v3 = v1.cross(v2);
	return std::make_pair(v2, v3);
    }

    // rotate around an arbitary axis clockwise
    // angle in radians
    template <template <typename, size_t> class Container>
    auto
    rotate(const real angle, const Vec3<Container> & axis) noexcept
    {
	const auto cos_theta = std::cos(angle);
	const auto sin_theta = std::sin(angle);

	const Mat3_ A(0, axis[2], -axis[1],
				    -axis[2], 0, axis[0],
				    axis[1], -axis[0], 0);

	const auto xy = axis[0]*axis[1];
	const auto xz = axis[0]*axis[2];
	const auto yz = axis[1]*axis[2];

	const Mat3_ aa(axis[0]*axis[0], xy, xz,
				     xy, axis[1]*axis[1], yz,
				     xz, yz, axis[2]*axis[2]);

	const auto rot = Mat3_{} * cos_theta
						   + aa * (real(1) - cos_theta)
						   + A * sin_theta;

	return Mat4_(rot);
    }
}
