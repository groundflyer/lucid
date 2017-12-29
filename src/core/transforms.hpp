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


    template <template <typename, size_t> class MatContainer,
			  template <typename, size_t> class VecContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
					const Vec3<VecContainer> & v) noexcept
    { return dehomogenize(t.dot(Vec4_(v))); }


    template <template <typename, size_t> class MatContainer,
			  template <typename, size_t> class PointContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
					const Point<PointContainer> & p) noexcept
    { return dehomogenize(t.dot(homogenize(p))); }

    template <template <typename, size_t> class MatContainer,
			  template <typename, size_t> class NormalContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
					const Normal<NormalContainer> & n) noexcept
    { return dehomogenize(transpose(inverse(t)).dot(Vec4_(n))); }

    template <template <typename, size_t> class MatContainer,
			  template <typename, size_t> class RayContainer>
    constexpr auto
    apply_transform(const Mat4<MatContainer> & t,
					const Ray<RayContainer> & ray)
    { return Ray<RayContainer>(apply_transform(t, ray.origin()), apply_transform(t, ray.dir())); }


    template <template <typename, size_t> class Container>
    constexpr auto
    translate(const Vec3<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4<Container>(1, 0, 0, x,
							   0, 1, 0, y,
							   0, 0, 1, z,
							   0, 0, 0, 1);
    }

    template <template <typename, size_t> class Container>
    constexpr auto
    scale(const Vec3<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4<Container>(x, 0, 0, 0,
							   0, y, 0, 0,
							   0, 0, z, 0,
							   0, 0, 0, 1);
    }

    template <template <typename, size_t> class EyeContainer,
			  template <typename, size_t> class CenContainer,
			  template <typename, size_t> class UpContainer>
    constexpr auto
    look_at(const Point<EyeContainer> & eye,
			const Point<CenContainer> & center,
			const Normal<UpContainer> & up) noexcept
    {
		const auto f = normalize(center - eye);
		const auto s = normalize(f.cross(up));
		const auto [ux, uy, uz] = s.cross(f);
		const auto& [sx, sy, sz] = s;
		const auto& [fx, fy, fz] = f;

		return Mat4_(sx, sy, sz, 0,
					 ux, uy, uz, 0,
					 -fx, -fy, -fz, 0,
					 0, 0, 0, 1).dot(translate(-eye));
    }


    template <template <typename, size_t> class Container>
    constexpr auto
    basis(const Vec3<Container> & v1) noexcept
    {
		const auto& [v1x, v1y, v1z] = v1;
		Vec3_ v2;
		auto& [v2x, v2y, v2z] = v2;

		if (std::abs(v1x) > std::abs(v1y))
		{
			const auto il = real(1) / (std::sqrt(v1x*v1x + v1z*v1z));
			v2x = -v1z * il;
			v2z = v1x * il;
		}
		else
		{
			const auto il = real(1) / (std::sqrt(v1y*v1y + v1z*v1z));
			v2y = v1z * il;
			v2z = -v1y * il;
		}

		return std::make_pair(v2, v1.cross(v2));
    }

    // rotate around an arbitary axis clockwise
    // angle in radians
    template <template <typename, size_t> class Container>
    auto
    rotate(const real angle, const Vec3<Container> & axis) noexcept
    {
		const auto cos_theta = std::cos(angle);
		const auto sin_theta = std::sin(angle);
		const auto& [x, y, z] = axis;

		const Mat3_ A(0, z, -y,
					  -z, 0, x,
					  y, -x, 0);

		const auto xy = x*y;
		const auto xz = x*z;
		const auto yz = y*z;

		const Mat3_ aa(x*x, xy, xz,
					   xy, y*y, yz,
					   xz, yz, z*z);

		const auto rot = Mat3_{} * cos_theta
									 + aa * (real(1) - cos_theta)
									 + A * sin_theta;

		return Mat4_(rot);
    }
}
