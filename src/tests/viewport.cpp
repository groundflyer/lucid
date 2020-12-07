// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>
#include <utils/functional.hpp>
#include <utils/logging.hpp>
#include <utils/timer.hpp>

using namespace lucid;
using namespace std::literals;

using Image    = ScanlineImage<float, 4>;
using FilmRGBA = Film<Image>;

static const auto                  room_geo   = CornellBox::geometry();
static const CornellBox::MatGetter mat_getter = CornellBox::mat_getter();

using Simple = Constant<std::decay_t<decltype(room_geo)>, CornellBox::MatGetter>;

FilmRGBA&
render(FilmRGBA& film, const perspective::shoot& cam) noexcept
{
    for(auto it = film.img.begin(); it != film.img.end(); ++it)
    {
        const Vec2 pp = film.sample_space(it.pos());
        Simple     ig{cam(pp), &room_geo, &mat_getter, pp};
        *it = RGBA(ig().second);
    }
    return film;
}

struct FilmCapture
{
    FilmRGBA* film;

    FilmRGBA&
    operator()(const Vec2i& res)
    {
        *film = FilmRGBA(Vec2u(res));
        return *film;
    }

    const Image&
    operator()(const perspective::shoot& cam) noexcept
    {
        *film = render(*film, cam);
        return film->img;
    }
};

struct CameraCapture
{
    perspective::shoot cam;

    Mat4
    get_transform(int key) const noexcept
    {
        auto tt = [&](const Vec3& t) noexcept { return dot(cam.transform, translate(t)); };
        switch(key)
        {
        case GLFW_KEY_UP: return tt(Vec3(0_r, 0.1_r, 0_r));
        case GLFW_KEY_DOWN: return tt(Vec3(0_r, -0.1_r, 0_r));
        }

        return cam.transform;
    }

    perspective::shoot
    operator()(int key, int action, int /*mods*/) noexcept
    {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) cam.transform = get_transform(key);
        return cam;
    }

    const Image&
    operator()(FilmRGBA& film) noexcept
    {
        film = render(film, cam);
        return film.img;
    }
};

// struct MouseAction
// {
//     Vec2 res;
//     Vec2 prev_pos{0_r};
//     bool pressed = false;

//     void
//     operator()(int button, bool _pressed, int /* mods */) noexcept
//     {
//         pressed = (button == GLFW_MOUSE_BUTTON_LEFT) && _pressed;
//     }

//     void
//     operator()(const Vec2& new_pos) noexcept
//     {
//         if (pressed)
//         {
//             const Vec2 pix_delta = new_pos - prev_pos;
//             const Vec2 ndc_delta = pix_delta / res;
//             const Vec2 angle_delta = lucid::transform(invert_fov, ndc_delta);
//          }
//     }
// };

struct DummyAction
{
    Image* img;

    template <typename... Args>
    const Image&
    operator()(Args&&...) const noexcept
    {
        return *img;
    }
};

int
main()
{
    Logger logger(Logger::DEBUG);
    logger.debug("Let's go");

    perspective::shoot cam = CornellBox::camera();

    const Vec2u res{1280, 720};
    FilmRGBA    film(res);

    CameraCapture cc{cam};
    FilmCapture   fc{&film};
    DummyAction   da{&film.img};
    auto          viewport = make_viewport(res, compose(cc, fc), compose(fc, cc), da);

    render(film, cam);
    viewport.load_img(film.img);

    logger.debug("Start main loop");
    while(viewport.active()) { viewport.draw(); }

    return 0;
}
