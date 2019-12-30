// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>

using namespace lucid;

static std::random_device                      rd;
static thread_local std::default_random_engine g(rd());


int main(int argc, char* argv[])
{
    const real radius{argc > 1 ? std::stof(argv[1]) : 2};

    Viewport::init(640, 640);

    Film<ScanlineImage<float, 4>> film(Viewport::get_res());

    const Vec2 res{Viewport::get_res()};
    const real filter_rad = film.pixel_radius() * radius;
    const PixelUpdate updater{TriangleFilter(filter_rad)};

    const auto& [w, h]       = Viewport::get_res();
    const real         ratio = static_cast<real>(w) / static_cast<real>(h);
    const perspective::shoot cam   = CornellBox::camera(ratio);
    const auto         room_geo   = CornellBox::geometry();
    const auto         mat_getter = CornellBox::mat_getter();
    using Simple = Constant<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;
    for(auto it = film.img.begin(); it != film.img.end(); ++it)
    {
        const Vec2 pp = film.device_coords(it.pos());
        const Vec2 sample_pos = sample_pixel(g, film.pixel_size, pp);
        Simple ig{cam(sample_pos), &room_geo, &mat_getter, sample_pos};
        const auto result = ig();
        update_pixels(film, updater, result);
    }

    Viewport::load_img(film.img);

    while(Viewport::active())
    {
        Viewport::draw();
        glfwPollEvents();
    }

    Viewport::cleanup();

    return 0;
}
