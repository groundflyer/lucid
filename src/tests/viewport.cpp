// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>
#include <utils/logging.hpp>
#include <utils/printing.hpp>
#include <utils/timer.hpp>

using namespace lucid;
using namespace std::literals;

static std::random_device                      rd;
static thread_local std::default_random_engine g(rd());

int
main()
{
    Logger logger(Logger::DEBUG);

    Viewport::init(Vec2u(640, 480));

    Film<ScanlineImage<float, 4>> film1{Vec2u(Viewport::get_res())};
    Film<ScanlineImage<float, 4>> film2{Vec2u(Viewport::get_res())};

    const real               hratio  = film1._ratio * 0.5_r;
    const real               hpwidth = film1._pixel_width * 0.5_r;
    RandomDistribution<real> hdist(-0.5_r - hpwidth, 0.5_r + hpwidth);
    RandomDistribution<real> wdist(-hratio - hpwidth, hratio + hpwidth);

    const auto        color_gen   = [&]() { return RGB{rand<float, 3>(g)}; };
    const auto        pos_gen     = [&]() { return Vec2(wdist(g), hdist(g)); };
    const unsigned    filter_size = 100u;
    const real        filter_rad  = film1._pixel_radius * filter_size;
    const PixelUpdate updater{TriangleFilter(filter_rad)};

    for(std::size_t i = 0; i < 30ul; ++i)
    {
        const Sample sample{pos_gen(), color_gen()};
        film1 = sample_based_region_update(film1, updater, sample);
    }

    const perspective::shoot cam        = CornellBox::camera();
    const auto               room_geo   = CornellBox::geometry();
    const auto               mat_getter = CornellBox::mat_getter();
    using Simple = Constant<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;
    for(auto it = film2.img.begin(); it != film2.img.end(); ++it)
    {
        const Vec2 pp = film1.sample_space(it.pos());
        Simple     ig{cam(pp), &room_geo, &mat_getter, pp};
        *it = RGBA(ig().second);
    }

    bool choice = true;
    Viewport::load_img(film1.img);

    ElapsedTimer<> timer;

    while(Viewport::active())
    {
        if(timer.has_expired(3s))
        {
            timer.restart();
            choice ^= true;

            if(choice)
            {
                fmt::print("Sample test\n");
                Viewport::load_img(film1.img);
            }
            else
            {
                fmt::print("Camera test\n");
                Viewport::load_img(film2.img);
            }
        }
        Viewport::draw();

        glfwPollEvents();
    }

    Viewport::cleanup();

    return 0;
}
