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
    { return Vec4(a[0], a[1], a[2], 1); }

    template <template <typename, size_t> typename Container>
    constexpr Vec3_<Container>
    dehomogenize(const Vec4_<Container> & a) noexcept
    {
        Vec3 ret(a);
        if(a[3] != 0 && a[3] != 1)
            ret /= a[3];
        return ret;
    }


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
    { return Point(dehomogenize(t.dot(homogenize(p)))); }

    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename NormalContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer> & t,
					const Normal_<NormalContainer> & n) noexcept
    { return Normal(dehomogenize(transpose(inverse(t)).dot(Vec4(n)))); }


    template <template <typename, size_t> typename Container>
    constexpr auto
    translate(const Vec3_<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4(1, 0, 0, x,
                    0, 1, 0, y,
                    0, 0, 1, z,
                    0, 0, 0, 1);
    }

    template <template <typename, size_t> typename Container>
    constexpr auto
    scale(const Vec3_<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4(x, 0, 0, 0,
                    0, y, 0, 0,
                    0, 0, z, 0,
                    0, 0, 0, 1);
    }

    // rotate around an arbitary axis clockwise
    // angle in radians
    template <template <typename, size_t> typename Container>
    constexpr auto
    rotate(const real angle, const Vec3_<Container> & axis) noexcept
    {
		const auto& [x, y, z] = axis;

		const Mat3 A(0, z, -y,
					 -z, 0, -x,
					 y, x, 0);

		const auto xy = x*y;
		const auto xz = x*z;
		const auto yz = y*z;

		const Mat3 aa(x*x, xy, xz,
					  xy, y*y, yz,
					  xz, yz, z*z);

		const auto cos_theta = math::cos(angle);
		const auto rot = Mat3::unit() * cos_theta +
			aa * (1_r - cos_theta) + A * math::sin(angle);

		return Mat4(rot);
    }

    template <template <typename, size_t> typename EyeContainer,
			  template <typename, size_t> typename CenContainer,
			  template <typename, size_t> typename UpContainer = std::array>
    constexpr auto
    look_at(const Point_<EyeContainer>& eye,
			const Point_<CenContainer>& target,
			const Normal_<UpContainer>& up = Normal_<UpContainer>(0, 1, 0)) noexcept
    {
		const auto f = normalize(eye - target);
		const auto s = normalize(up.cross(f));
        const auto u = f.cross(s);
		return inverse(Mat4(Vec4(s), Vec4(u), Vec4(f), Vec4(0,0,0,1)).dot(translate(-eye)));
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
}