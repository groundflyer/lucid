// -*- C++ -*-
// transforms.hpp --
//

#pragma once

#include <core/basic_types.hpp>
#include <utility>
#include <cmath>


namespace yapt
{
    constexpr Vec4
    homogenize_point(const Vec3 & a) noexcept
    {
	Vec4 ret (a);
	ret[3] = 1;
	return ret;
    }

    constexpr Vec3
    dehomogenize(const Vec4 & a) noexcept
    { return Vec3(a) / (a.w() != 0 ? a.w() : 1); }

    constexpr Vec3
    apply_transform_point(const Mat4 & t, const Vec3 & a) noexcept
    { return dehomogenize(t * homogenize_point(a)); }

    constexpr Vec3
    apply_transform_vector(const Mat4 & t, const Vec3 & a) noexcept
    { return dehomogenize(t * Vec4(a)); }

    constexpr Vec3
    apply_transform_normal(const Mat4 & t, const Vec3 & n) noexcept
    { return dehomogenize(transpose(inverse(t)) * Vec4(n)); }


    class Transform
    {
	Mat4 _t;
    public:
	constexpr
	Transform() {}

	explicit constexpr
	Transform(const Mat4 & t) : _t(t) {}

	constexpr void
	push(const Mat4 & t) noexcept
	{ _t *= t; }

	// default behavior is to transform as vector
	constexpr Vec3
	operator()(const Vec3 & v) const noexcept
	{ return dehomogenize(_t * Vec4(v)); }

	constexpr Vec3
	as_point(const Vec3 & p) const noexcept
	{ return dehomogenize(_t * homogenize_point(p)); }

	constexpr Vec3
	as_normal(const Vec3 & n) const noexcept
	{ return dehomogenize(transpose(inverse(_t)) * Vec4(n)); }
    };


    constexpr std::pair<Vec3, Vec3>
    make_basis(const Vec3 & v1) noexcept
    {
	Vec3 v2;

	if (std::abs(v1.x()) > std::abs(v1.y()))
	    {
		real il = 1 / (std::sqrt(v1.x()*v1.x() + v1.z()*v1.z()));

		v2 = Vec3(-v1.z() * il, 0, v1.x()* il);
	    }
	else
	    {
		real il = 1 / (std::sqrt(v1.y()*v1.y() + v1.z()*v1.z()));

		v2 = Vec3(0, v1.z() * il, -v1.y()* il);
	    }

	auto v3 = v1 ^ v2;

	return std::make_pair(v2, v3);
    }


    constexpr Mat4
    translate(const Vec3 & delta) noexcept
    {
	return Mat4(1, 0, 0, delta.x(),
		    0, 1, 0, delta.y(),
		    0, 0, 1, delta.z(),
		    0, 0, 0, 1);
    }

    constexpr Mat4
    scale(const Vec3 & delta) noexcept
    {
	return Mat4(delta.x(), 0, 0, 0,
		    0, delta.y(), 0, 0,
		    0, 0, delta.z(), 0,
		    0, 0, 0, 1);
    }

    // rotate around an arbitary axis clockwise
    // angle in radians
    constexpr Mat4
    rotate(const real angle, const Vec3 & axis) noexcept
    {
	const auto cos_theta = std::cos(angle);
	const auto sin_theta = std::sin(angle);

	const Mat3 A = Mat3(0, axis.z(), -axis.y(),
			    -axis.z(), 0, axis.x(),
			    axis.y(), -axis.x(), 0);

	const auto xy = axis.x()*axis.y();
	const auto xz = axis.x()*axis.z();
	const auto yz = axis.y()*axis.z();

	const Mat3 aa = Mat3(axis.x()*axis.x(), xy, xz,
			     xy, axis.y()*axis.y(), yz,
			     xz, yz, axis.z()*axis.z());

	const Mat3 rot = Mat3{} * cos_theta
				    + aa * (real(1) - cos_theta)
				    + A * sin_theta;

	return Mat4(rot);
    }

    constexpr Mat4
    look_at(const Vec3 & eye, const Vec3 & center, const Vec3 & up)
    {
	const Vec3 f = normalize(center - eye);
	Vec3 u = normalize(up);
	const Vec3 s = normalize(f ^ u);
	u = s ^ f;

	const Mat4 ret = Mat4(s.x(), s.y(), s.z(), 0,
			      u.x(), u.y(), u.z(), 0,
			      -f.x(), -f.y(), -f.z(), 0,
			      0, 0, 0, 1);

	return ret * translate(-eye);
    }
}
