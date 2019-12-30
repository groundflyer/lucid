// -*- C++ -*-
// filtering.cpp
#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <sampling/film.hpp>

#include <numeric>

using namespace lucid;

template <typename Image, typename Updater>
Film<Image>&
simple_update(Film<Image>& film, Updater&& update, const Sample& sample) noexcept
{
    for(auto it = film.img.begin(); it != film.img.end(); ++it)
    {
        const Vec2 pp = film.device_coords(it.pos());
        *it           = update(*it, pp, sample);
    }
    return film;
}

int main()
{
    const Vec2u res(400, 400);
    Film<ScanlineImage<float, 4>> film(res);
    const real filter_rad = film.pixel_radius() * 300;
    const PixelUpdate updater{TriangleFilter(filter_rad)};

    const Sample samples[]{Sample{Vec2(-0.25_r), RGB(0.2_r, 1_r, 0.1_r)},
                           Sample{Vec2(0.25_r), RGB(1_r, 0.2_r, 0.1_r)},
                           Sample{Vec2(-0.25_r, 0.25_r), RGB(0.1_r, 0.9_r, 1_r)},
                           Sample{Vec2(0.25_r, -0.25_r), RGB(1_r, 0.9_r, 0.1_r)}};

    auto film_validate = film;

    for (const Sample& sample : samples)
    {
        film = update_pixels(film, updater, sample);
        film_validate = simple_update(film_validate, updater, sample);
    }

    auto iter2 = film_validate.img.begin();
    int ret = 0;
    for (const auto val : film.img)
        ret += all(val != *(iter2++));
    
    return ret;
}
