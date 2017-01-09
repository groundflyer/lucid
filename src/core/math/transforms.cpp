// -*- C++ -*-
// transforms.cpp --

#include "transforms.hpp"

namespace yapt
{
    std::pair<Vec3, Vec3>
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

	return std::pair<Vec3, Vec3>(v2, v3);
    }

    Mat4
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
}
