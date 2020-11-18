// -*- C++ -*-
// basic.hpp
//

#pragma once

#include <image_reconstruction/filtering.hpp>
#include <primitives/primitives.hpp>
#include <ray_traversal/ray_traversal.hpp>
#include <utils/tuple.hpp>

#include <random>

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
        const auto [pid, isect]    = hider(ray, *scene);
        const auto& [bsdf, emit_f] = (*material_getter)(pid);
        const Vec3 pos             = hit_pos(ray, isect);
        const Vec3 n               = lucid::visit(
            pid, [&](const auto& prim) noexcept { return normal(pos, prim); }, *scene);
        const auto eval     = bsdf(n).first;
        const RGB  color    = eval(n, n);
        const RGB  emission = emit_f();
        const RGB  ret      = isect ? color + emission : RGB(0_r);
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

    Sample
    operator()() noexcept
    {
        RGB  radiance{1};
        bool has_rad = false;

        for(std::size_t depth = 0; depth < max_depth; ++depth)
        {
            const auto& [ro, wo]    = ray;
            const auto [pid, isect] = hider(ray, *scene);

            if(!isect) break;

            const auto& [bsdf, emit_f] = (*material_getter)(pid);
            const Vec3 pos             = hit_pos(ray, isect);

            const Vec3 n = lucid::visit(
                pid, [&](const auto& prim) noexcept { return normal(pos, prim); }, *scene);

            const auto [eval, sample] = bsdf(n);

            const Vec3 wi =
                normalize(sample(wo,
                                 Vec2(generate<2>(static_cast<float (*)(RandomEngine&)>(
                                                      std::generate_canonical<float, 8>),
                                                  *g))));
            const RGB color    = eval(wi, wo);
            const RGB emission = emit_f();

            has_rad |= any(emission > 0_r);

            if(all(almost_equal(color, 0_r, 10)))
            {
                radiance *= emission;
                break;
            }

            const Vec3 p = ro + wo * isect.t;

            radiance = radiance * color + emission;

            ray = Ray(p + wi * bias, wi);
        }

        return Sample{sample_pos, radiance * has_rad};
    }
};
} // namespace lucid
