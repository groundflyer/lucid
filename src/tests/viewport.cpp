// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>
#include <utils/functional.hpp>
#include <utils/logging.hpp>
#include <utils/printing.hpp>
#include <utils/timer.hpp>

using namespace lucid;
using namespace std::literals;

using Image    = ScanlineImage<float, 4>;
using FilmRGBA = Film<Image>;

struct Renderer
{
    static inline const auto                  room_geo   = CornellBox::geometry();
    static inline const CornellBox::MatGetter mat_getter = CornellBox::mat_getter();

    using Simple = Constant<std::decay_t<decltype(room_geo)>, CornellBox::MatGetter>;

    const perspective::shoot* cam;

    const Image&
    operator()(FilmRGBA& film) noexcept
    {
        for(auto it = film.img.begin(); it != film.img.end(); ++it)
        {
            const Vec2 pp = film.sample_space(it.pos());
            Simple     ig{(*cam)(pp), &room_geo, &mat_getter, pp};
            *it = RGBA(ig().second);
        }

        return film.img;
    }
};

struct ResizeFilm
{
    FilmRGBA* film;

    FilmRGBA&
    operator()(const Vec2i& res)
    {
        *film = FilmRGBA(Vec2u(res));
        return *film;
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

    perspective::shoot cam = CornellBox::camera();

    const Vec2u res{320, 240};
    FilmRGBA    film(res);
    Renderer    render{&cam};

    auto viewport = make_viewport(res, compose(render, ResizeFilm{&film}), DummyAction{&film.img});

    render(film);
    viewport.load_img(film.img);

    while(viewport.active())
    {
        viewport.draw();
        glfwPollEvents();
    }

    return 0;
}
