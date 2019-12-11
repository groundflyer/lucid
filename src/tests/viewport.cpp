// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
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

    vp::init(640, 640);

    ScanlineImage<float, 4> img(vp::res);

    const Vec2 res{vp::res};
    const Vec2 pixel_size = Vec2(1_r) / res;
    const real filter_rad = pixel_radius(pixel_size) * radius;
    const PixelUpdate updater{TriangleFilter(filter_rad)};

    // const auto& [w, h]       = vp::res;
    // const real         ratio = static_cast<real>(w) / static_cast<real>(h);
    // perspective::shoot cam   = make_cam(ratio);
    // const auto         room_geo   = make_room();
    // const auto         mat_getter = get_mat_getter();
    // using Simple = Constant<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;
    // for(auto it = img.begin(); it != img.end(); ++it)
    // {
    //     const Vec2 pp = device_coords(it.pos(), res, pixel_size);
    //     // const Vec2 sample_pos = sample_pixel(g, pixel_size, pp);
    //     Simple ig{cam(pp), &room_geo, &mat_getter, pp};
    //     const auto result = ig();
    //     *it = RGBA(result.second);
    //     // update_pixels(img, updater, res, pixel_size, result);
    // }

    const Sample samples[]{Sample{Vec2(-0.25_r), RGB(0.2_r, 1_r, 0.1_r)},
                           Sample{Vec2(0.25_r), RGB(1_r, 0.2_r, 0.1_r)},
                           Sample{Vec2(-0.25_r, 0.25_r), RGB(0.1_r, 0.9_r, 1_r)},
                           Sample{Vec2(0.25_r, -0.25_r), RGB(1_r, 0.9_r, 0.1_r)}};

    for (const Sample& sample : samples)
        update_pixels(img, updater, res, pixel_size, sample);

    vp::load_img(img);

    while(vp::active())
    {
        vp::draw();
        glfwPollEvents();
    }

    vp::cleanup();

    return 0;
}
