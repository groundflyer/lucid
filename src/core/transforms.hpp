// -*- C++ -*-
// transforms.hpp --
//

#pragma once

#include "types.hpp"

#include <utility>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    constexpr Vec4_<Container>
    homogenize(const Point_<Container> & a) noexcept
    {
		Vec4_<Container> ret(a);
		ret[3] = 1;
		return ret;
    }

    template <template <typename, size_t> typename Container>
    constexpr Vec3_<Container>
    dehomogenize(const Vec4_<Container> & a) noexcept
    { return Vec3_<Container>(a) / (a[3] != 0 ? a[3] : 1); }


    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename VecContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer> & t,
					const Vec3_<VecContainer> & v) noexcept
    { return dehomogenize(t.dot(Vec4(v))); }


    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename PointContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer> & t,
					const Point_<PointContainer> & p) noexcept
    { return dehomogenize(t.dot(homogenize(p))); }

    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename NormalContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer> & t,
					const Normal_<NormalContainer> & n) noexcept
    { return dehomogenize(transpose(inverse(t)).dot(Vec4(n))); }


    template <template <typename, size_t> typename Container>
    constexpr auto
    translate(const Vec3_<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4_<Container>(1, 0, 0, x,
								0, 1, 0, y,
								0, 0, 1, z,
								0, 0, 0, 1);
    }

    template <template <typename, size_t> typename Container>
    constexpr auto
    scale(const Vec3_<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4_<Container>(x, 0, 0, 0,
								0, y, 0, 0,
								0, 0, z, 0,
								0, 0, 0, 1);
    }

    template <template <typename, size_t> typename EyeContainer,
			  template <typename, size_t> typename CenContainer,
			  template <typename, size_t> typename UpContainer>
    constexpr auto
    look_at(const Point_<EyeContainer> & eye,
			const Point_<CenContainer> & center,
			const Normal_<UpContainer> & up) noexcept
    {
		const auto f = normalize(center - eye);
		const auto s = normalize(f.cross(up));
		const auto [ux, uy, uz] = s.cross(f);
		const auto& [sx, sy, sz] = s;
		const auto& [fx, fy, fz] = f;

		return Mat4(sx, sy, sz, 0,
					ux, uy, uz, 0,
					-fx, -fy, -fz, 0,
					0, 0, 0, 1).dot(translate(-eye));
    }


    template <template <typename, size_t> typename Container>
    constexpr auto
    basis(const Vec3_<Container> & v1) noexcept
    {
		const auto& [v1x, v1y, v1z] = v1;
		Vec3 v2;
		auto& [v2x, v2y, v2z] = v2;

		if(math::abs(v1x) > math::abs(v1y))
		{
			const auto il = real(1) / (math::sqrt(v1x*v1x + v1z*v1z));
			v2x = -v1z * il;
			v2z = v1x * il;
		}
		else
		{
			const auto il = real(1) / (math::sqrt(v1y*v1y + v1z*v1z));
			v2y = v1z * il;
			v2z = -v1y * il;
		}

		return std::pair(v2, v1.cross(v2));
    }

    // rotate around an arbitary axis clockwise
    // angle in radians
    template <template <typename, size_t> typename Container>
    constexpr auto
    rotate(const real angle, const Vec3_<Container> & axis) noexcept
    {
		const auto& [x, y, z] = axis;

		const Mat3 A(0, -z, y,
					 z, 0, -x,
					 -y, x, 0);

		const auto xy = x*y;
		const auto xz = x*z;
		const auto yz = y*z;

		const Mat3 aa(x*x, xy, xz,
					  xy, y*y, yz,
					  xz, yz, z*z);

		const auto cos_theta = math::cos(angle);
		const auto rot = Mat3::unit() * cos_theta +
			aa * (1 - cos_theta) + A * math::sin(angle);

		return Mat4_<Container>(rot);
    }
}
