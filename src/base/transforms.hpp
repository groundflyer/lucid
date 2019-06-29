// -*- C++ -*-
// transforms.hpp --
//

#pragma once

#include "types.hpp"

#include <utility>


namespace lucid
{
    template <template <typename, size_t> typename Container>
    constexpr auto
    dehomogenize(const Vec4_<Container> & a) noexcept
    {
        Vec3 ret{a};
        const auto& w = std::get<3>(a);
        if(!(almost_equal(w, 0_r, 5) || almost_equal(w, 1_r, 5)))
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
    { return Point(dehomogenize(t.dot(Vec4(p, 1)))); }

    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename NormalContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer> & t,
					const Normal_<NormalContainer> & n) noexcept
    { return Normal(dehomogenize(transpose(inverse(t)).dot(Vec4(n)))); }


    template <template <typename, size_t> typename Container>
    constexpr auto
    translate(const Point_<Container> & delta) noexcept
    {
		const auto& [x, y, z] = delta;
		return Mat4(1, 0, 0, x,
                    0, 1, 0, y,
                    0, 0, 1, z,
                    0, 0, 0, 1);
    }

    template <template <typename, size_t> typename Container>
    constexpr auto
    scale(const Point_<Container> & delta) noexcept
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
    rotate(const real angle, const Normal_<Container> & axis) noexcept
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
		const Normal f(eye - target);
		const auto r = up.cross(f);
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
		return std::pair(Normal(1_r + sign * pow<2>(nx) * a,
                                sign * b,
                                -sign * nx),
                         Normal(b,
                                sign + pow<2>(ny) * a,
                                -ny));
    }

    template <template <typename, size_t> typename Container>
    constexpr auto
    basis_matrix(const Normal_<Container>& z) noexcept
    {
        const auto [x, y] = basis(z);
        return transpose(Mat3(x, y, z));
    }
}
