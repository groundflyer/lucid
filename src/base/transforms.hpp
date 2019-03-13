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
    { return Vec4(a, 1_r); }

    template <template <typename, size_t> typename Container>
    constexpr Vec3_<Container>
    dehomogenize(const Vec4_<Container> & a) noexcept
    {
        Vec3 ret{a};
        const auto& w = std::get<3>(a);
        if(w != 0_r && w != 1_r)
            ret /= w;
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

        const Mat3 K{0, -z, y,
                     z, 0, -x,
                     -y, x, 0};

        return Mat4(Mat3::identity() + K * math::sin(angle) + K.dot(K) * (1_r - math::cos(angle)));
    }

    template <template <typename, size_t> typename EyeContainer,
			  template <typename, size_t> typename CenContainer,
			  template <typename, size_t> typename UpContainer>
    constexpr auto
    look_at(const Point_<EyeContainer>& eye,
			const Point_<CenContainer>& target,
			const Normal_<UpContainer>& up) noexcept
    {
		const auto f = normalize(eye - target);
		const auto r = normalize(up.cross(f));
        const auto u = f.cross(r);
		return inverse(Mat4(r, 0_r,
                            u, 0_r,
                            f, 0_r,
                            0_r, 0_r, 0_r, 1_r).dot(translate(-eye)));
    }


    template <template <typename, size_t> typename Container>
    constexpr auto
    basis(const Normal_<Container>& n) noexcept
    {
		const auto& [nx, ny, nz] = n;
        const auto sign = std::copysign(1_r, nz);
        const auto a = -1_r / (sign + nz);
        const auto b = nx * ny * a;
		return std::pair(Normal(1_r + sign * math::pow<2>(nx) * a,
                                sign * b,
                                -sign * nx),
                         Normal(b,
                                sign + math::pow<2>(ny) * a,
                                -ny));
    }
}
