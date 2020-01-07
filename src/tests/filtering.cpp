// -*- C++ -*-
// filtering.cpp
#include "property_test.hpp"

#include <base/rng.hpp>
#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <sampling/film.hpp>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

using namespace lucid;
using namespace ranges;

template <typename Image, typename Updater>
Film<Image>&
full_update(Film<Image>& film, Updater&& update, const Sample& sample) noexcept
{
    for(auto it = film.img.begin(); it != film.img.end(); ++it)
    {
        const Vec2 pp = film.device_coords(it.pos());
        *it           = update(*it, pp, sample);
    }
    return film;
}

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? std::stoul(argv[1]) : 100;

    std::random_device rd;
    std::default_random_engine g(rd());
    RandomDistribution<real> pos_dist(-0.5_r, 0.5_r);
    RandomDistribution<unsigned> count_dist(3u, 400u);

    const auto color_gen = [&](){ return RGB{rand<float, 3>(g)}; };
    const auto pos_gen = [&](){ return Vec2(pos_dist.template operator()<2>(g)); };
    const auto res_gen = [&](){ return Vec2u(count_dist.template operator()<2>(g)); };

    const auto generator = [&](){ return std::tuple{res_gen(), count_dist(g), views::generate_n([&](){ return Sample{pos_gen(), color_gen()}; }, count_dist(g))}; };
    const auto testing = [](auto&& feed)
                         {
                             auto&& [res, filter_size, samples] = feed;
                             Film<ScanlineImage<float, 4>> film_test(res);
                             Film<ScanlineImage<float, 4>> film_valid(res);
                             const real filter_rad = film_test.pixel_radius() * filter_size;
                             const PixelUpdate updater{TriangleFilter(filter_rad)};
                             for_each(samples, [&](const Sample& sample)
                             {
                                 film_test = update_pixels(film_test, updater, sample);
                                 film_valid = full_update(film_valid, updater, sample);
                             });
                             return std::pair{std::move(film_test), std::move(film_valid)};
                         };
    const auto property = [](const auto& tested, const auto&)
                          {
                              const auto& [film_test, film_valid] = tested;
                              auto iter2 = film_valid.img.begin();
                              int ret = 0;
                              for (const auto val : film_test.img)
                                  ret += all(!almost_equal(val, *(iter2++), 100u));
                              return ret;
                          };

    const bool ret = test_property(num_tests, 0.05, "Update Pixels",
                                   generator, testing, property);
    
    return ret;
}
