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
        const auto [pid, isect]    = hider(ray, *scene);
        const auto& [bsdf, emit_f] = (*material_getter)(pid);
        const Normal n             = lucid::visit(
            pid,
            [&, &iss = isect](const auto& prim) noexcept { return normal(ray, iss, prim); },
            *scene);
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

            const Normal n = lucid::visit(
                pid,
                [&, &iss = isect](const auto& prim) noexcept { return normal(ray, iss, prim); },
                *scene);

            const auto [eval, sample] = bsdf(n);

            const Normal wi       = sample(wo, Vec2(rand<real, 2>(*g)));
            const RGB    color    = eval(wi, wo);
            const RGB    emission = emit_f();

            has_rad |= any(emission > 0_r);

            if(all(almost_equal(color, 0_r, 10)))
            {
                radiance *= emission;
                break;
            }

            const Point p = ro + wo * isect.t;

            radiance = radiance * color + emission;

            ray = Ray(p + wi * bias, wi);
        }

        return Sample{sample_pos, radiance * has_rad};
    }
};
} // namespace lucid
