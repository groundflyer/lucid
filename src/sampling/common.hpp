// -*- C++ -*-
// common.hpp
//

#pragma once

#include <base/transforms.hpp>
#include <base/types.hpp>
#include <utils/math.hpp>

namespace lucid
{
Vec3
sample_hemisphere(const Vec3& n, const Vec2& u) noexcept
{
    const auto& [u1, u2] = u;
    const real r         = 2_r * Pi * u2;
    const real phi       = math::sqrt(1_r - pow<2>(u1));
    return dot(basis_matrix(n), (Vec3(math::cos(r) * phi, math::sin(r) * phi, u1)));
}
} // namespace lucid
