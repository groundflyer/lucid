// -*- C++ -*-
// basic.hpp
//

#pragma once

#include <base/rng.hpp>
#include <image_reconstruction/filtering.hpp>
#include <ray_traversal/ray_traversal.hpp>
#include <utils/tuple.hpp>

namespace lucid
{
template <typename Scene, typename MaterialGetter>
struct Constant
{
    Ray                   ray;
    Scene const*          scene;
    MaterialGetter const* material_getter;
    Vec2                  sample_pos;

    Sample
    operator()() const noexcept
    {
        const auto [isect, pid]       = hider(ray, *scene);
        const auto& [color, emission] = (*material_getter)(pid);
        const RGB ret                 = isect ? color + emission : RGB(0_r);
        return Sample{sample_pos, ret};
    }
};

template <typename RandomEngine, typename Scene, typename MaterialGetter>
struct PathTracer_
{
    RandomEngine*         g;
    Scene const*          scene;
    MaterialGetter const* material_getter;
    Ray                   ray;
    std::uint8_t          max_depth;
    real                  bias;
    Vec2                  sample_pos;

    static Vec3
    sample_hemisphere(const Normal& n, const Vec2& u) noexcept
    {
        const auto& [u1, u2] = u;
        const auto r         = 2_r * Pi * u2;
        const auto phi       = math::sqrt(1_r - pow<2>(u1));
        return basis_matrix(n).dot(Vec3(math::cos(r) * phi, math::sin(r) * phi, u1));
    }

    Sample
    operator()() noexcept
    {
        RGB  radiance{1};
        bool has_rad = false;

        for(std::size_t depth = 0; depth < max_depth; ++depth)
        {
            const auto& [ro, rd]    = ray;
            const auto [isect, pid] = hider(ray, *scene);

            if(!isect) break;

            const auto& [color, emission] = (*material_getter)(pid);
            has_rad |= any(emission > 0_r);

            if(all(almost_equal(color, 0_r, 10)))
            {
                radiance *= emission;
                break;
            }

            const Normal n = lucid::visit(
                pid,
                [&, &iss = isect](const auto& prim) { return normal(ray, iss, prim); },
                *scene);

            const Point  p = ro + rd * isect.t;
            const Normal new_dir(sample_hemisphere(n, Vec2(rand<real, 2>(*g))));

            radiance = radiance * color * std::max(dot(n, new_dir), 0_r) + emission;

            ray = Ray(p + new_dir * bias, new_dir);
        }

        return Sample{sample_pos, radiance * has_rad};
    }
};
} // namespace lucid
