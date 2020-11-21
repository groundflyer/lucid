// -*- C++ -*-
// filtering.cpp
#include "property_test.hpp"

#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <sampling/film.hpp>
#include <utils/tuple.hpp>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/generate_n.hpp>

#include <random>

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
    std::random_device                      rd;
    std::default_random_engine              g(rd());
    std::uniform_int_distribution<unsigned> count_dist(10u, 200u);

    const auto color_gen = [&]() noexcept {
        return RGB{generate<3>(
            static_cast<float (*)(std::default_random_engine&)>(std::generate_canonical<float, 8>),
            g)};
    };
    const auto rand =
        static_cast<real (*)(std::default_random_engine&)>(std::generate_canonical<float, 8>);
    const auto pos_gen = [&](const real _ratio) noexcept {
        return Vec2{_ratio * (rand(g) - 0.5_r), rand(g) - 0.5_r};
    };
    const auto res_gen = [&]() noexcept { return Vec2u(generate<2>(count_dist, g)); };

    int ret = 0;

    ret += test_property(
        1000000ul,
        .01,
        "Sample space and pixel position",
        [&]() noexcept {
            const Vec2u res{res_gen()};
            const auto [w, h] = res;
            std::uniform_int_distribution<unsigned> distw(0u, w - 1u);
            std::uniform_int_distribution<unsigned> disth(0u, h - 1u);
            return std::pair{Vec2(res), Vec2u(distw(g), disth(g))};
        },
        [](const Vec2& res, const Vec2u& idx) noexcept { return sample_space(res, idx); },
        [](const Vec2& fss, const auto& res_idx) noexcept {
            const auto& [res, idxg] = res_idx;
            const auto& [fsx, fsy]  = fss;
            const Vec2u idxt        = pixel_index(res, fss);
            // this check fails in some cases with -O3 -ffast-math
            const bool error_prone = lucid::abs(fsx) > ratio(res) * 0.5_r;
            return any(idxg != idxt) || error_prone || (lucid::abs(fsy) > 0.5_r);
        });

    const auto generator = [&]() noexcept {
        const Vec2u res    = res_gen();
        const real  _ratio = ratio(res);
        return std::tuple{res,
                          count_dist(g),
                          views::generate_n(
                              [&, _ratio]() noexcept {
                                  return Sample{pos_gen(_ratio), color_gen()};
                              },
                              count_dist(g))};
    };
    const auto testing = [](const Vec2u& res, const unsigned filter_size, auto samples) noexcept {
        Film<ScanlineImage<float, 4>> film_test(res);
        Film<ScanlineImage<float, 4>> film_valid(res);
        const real filter_rad = film_test._pixel_radius * std::min(filter_size, lucid::min(res));
        const PixelUpdate updater{TriangleFilter(filter_rad)};
        ranges::for_each(samples, [&](const Sample& sample) noexcept {
            film_test  = sample_based_region_update(film_test, updater, sample);
            film_valid = full_update(film_valid, updater, sample);
        });
        return std::pair{std::move(film_test), std::move(film_valid)};
    };
    const auto property = [](const auto& tested, const auto&) noexcept {
        auto&& [film_test, film_valid] = tested;
        auto iter2                     = film_valid.img.begin();
        int  ret                       = 0;
        for(const auto val: film_test.img) ret += all(!almost_equal(val, *(iter2++), 100u));
        return ret;
    };

    ret +=
        test_property(100, 0.05, "Sample-based regional film update", generator, testing, property);

    return ret;
}
