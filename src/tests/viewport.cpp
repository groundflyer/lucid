// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>

using namespace lucid;

static std::random_device                      rd;
static thread_local std::default_random_engine g(rd());


int main(int argc, char* argv[])
{
    const unsigned short width     = argc > 1 ? std::stoi(argv[1]) : 640u;
    const unsigned short height    = argc > 2 ? std::stoi(argv[2]) : 480u;

    Viewport::init(width, height);

    Film<ScanlineImage<float, 4>> film{Vec2u(Viewport::get_res())};

    const auto& [w, h]       = Viewport::get_res();
    const real         ratio = static_cast<real>(w) / static_cast<real>(h);
    const perspective::shoot cam   = CornellBox::camera(ratio);
    const auto         room_geo   = CornellBox::geometry();
    const auto         mat_getter = CornellBox::mat_getter();
    using Simple = Constant<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;
    for(auto it = film.img.begin(); it != film.img.end(); ++it)
    {
        const Vec2 pp = film.device_coords(it.pos());
        Simple ig{cam(pp), &room_geo, &mat_getter, pp};
        *it = RGBA(ig().second);
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
