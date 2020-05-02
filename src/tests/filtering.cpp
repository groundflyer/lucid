// -*- C++ -*-
// filtering.cpp
#include "property_test.hpp"

#include <base/rng.hpp>
#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <sampling/film.hpp>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/generate_n.hpp>

using namespace lucid;
using namespace ranges;

template <typename Image, typename Updater>
Film<Image>&
full_update(Film<Image>& film, Updater&& update, const Sample& sample) noexcept
{
    for(auto it = film.img.begin(); it != film.img.end(); ++it)
        *it = update(*it, film.sample_space(it.pos()), sample);
    return film;
}

int
main()
{
    std::random_device           rd;
    std::default_random_engine   g(rd());
    RandomDistribution<unsigned> count_dist(3u, 400u);

    const auto color_gen = [&]() { return RGB{rand<float, 3>(g)}; };
    const auto pos_gen   = [&](const real _ratio,
                               const real _pixel_width) {
                               const real hratio = _ratio * 0.5_r;
                               const real hpwidth = _pixel_width * 0.5_r;
                               RandomDistribution<real> hdist(-0.5_r - hpwidth, 0.5_r + hpwidth);
                               RandomDistribution<real> wdist(-hratio - hpwidth, hratio + hpwidth);
                               return Vec2(wdist(g), hdist(g));
                           };
    const auto res_gen   = [&]() { return Vec2u(count_dist.template operator()<2>(g)); };

    int ret = 0;

    ret += test_property(
        1000000ul,
        .0006,
        "sample space and pixel position",
        [&]() {
            const Vec2u res{res_gen()};
            const auto [w, h] = res;
            RandomDistribution<unsigned> distw(0u, w - 1u);
            RandomDistribution<unsigned> disth(0u, h - 1u);
            return std::pair{Vec2(res), Vec2u(distw(g), disth(g))};
        },
        [](const auto& res_idx) {
            const auto& [res, idx] = res_idx;
            return sample_space(res, idx);
        },
        [](const Vec2& fss, const auto& res_idx) {
            const auto& [res, idxg] = res_idx;
            const auto& [fsx, fsy]  = fss;
            const Vec2u idxt        = pixel_index(res, fss);
            // this check fails approximately in 0.0055% cases with -O3 -ffast-math
            const bool error_prone = math::fabs(fsx) > ratio(res) * 0.5_r;
            return any(idxg != idxt) || error_prone || (math::abs(fsy) > 0.5_r);
        });

    const auto generator = [&]() {
                               const Vec2u res = res_gen();
        return std::tuple{res,
                          count_dist(g),
                          views::generate_n(
                              [&]() {
                                  return Sample{pos_gen(ratio(res), pixel_width(res)), color_gen()};
                              },
                              count_dist(g))};
    };
    const auto testing = [](auto&& feed) {
        auto&& [res, filter_size, samples] = feed;
        Film<ScanlineImage<float, 4>> film_test(res);
        Film<ScanlineImage<float, 4>> film_valid(res);
        const real filter_rad = film_test._pixel_radius * std::min(filter_size, lucid::min(res));
        const PixelUpdate updater{TriangleFilter(filter_rad)};
        for_each(samples, [&](const Sample& sample) {
            film_test  = sample_based_update(film_test, updater, sample);
            film_valid = full_update(film_valid, updater, sample);
        });
        return std::pair{std::move(film_test), std::move(film_valid)};
    };
    const auto property = [](auto&& tested, const auto&) {
        auto&& [film_test, film_valid] = tested;
        auto iter2                     = film_valid.img.begin();
        int  ret                       = 0;
        for(const auto val: film_test.img) ret += all(!almost_equal(val, *(iter2++), 100u));
        return ret;
    };

    ret += test_property(100, 0.05, "Sample-based regional film update", generator, testing, property);

    return ret;
}
