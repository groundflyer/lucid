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

    // can't be constant expressions
    std::pair<Vec3, Vec3>
    make_basis(const Vec3 & v1) noexcept;

    // rotate around an arbitary axis clockwise
    // angle in radians
    Mat4
    rotate(const real angle, const Vec3 & axis) noexcept;
}
