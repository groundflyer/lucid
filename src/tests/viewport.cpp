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

// static std::random_device                      rd;
// static thread_local std::default_random_engine g(rd());

struct MouseAction;
struct ResizeAction;

using Image    = ScanlineImage<float, 4>;
using FilmRGBA = Film<Image>;

struct Renderer
{
    static const inline auto                  room_geo   = CornellBox::geometry();
    static const inline CornellBox::MatGetter mat_getter = CornellBox::mat_getter();

    using Simple = Constant<std::decay_t<decltype(room_geo)>, CornellBox::MatGetter>;

    FilmRGBA film;

    explicit Renderer(const Vec2u& res) noexcept : film(res) {}

    void
    operator()(const perspective::shoot& cam) noexcept
    {
        for(auto it = film.img.begin(); it != film.img.end(); ++it)
        {
            const Vec2 pp = film.sample_space(it.pos());
            Simple     ig{cam(pp), &room_geo, &mat_getter, pp};
            *it = RGBA(ig().second);
        }
    };
};

struct ResizeAction
{
    Renderer* render;

    void
    operator()(const Vec2i& res)
    {
        render->film = FilmRGBA(Vec2u(res));
    }
};

// struct MouseAction
// {
//     perspective::shoot* cam;
//     Renderer* render;
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
//             const Normal axis = cross(Normal(0_r, 0_r, -1_r), Normal(angle_delta, 0_r));
//             const real angle = length(angle_delta);
//             if (angle > 0.001_r)
//             {
//                 cam->transform = dot(cam->transform, rotate(angle, axis));
//                 (*render)(*cam);
//                 prev_pos = new_pos;
//                 Viewport::load_img(render->film.img);
//             }
//         }
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
    Renderer    render(res);

    auto dummy_action = DummyAction{&render.film.img};
    auto viewport     = make_viewport(res, dummy_action, dummy_action);

    render(cam);
    viewport.load_img(render.film.img);

    while(viewport.active())
    {
        viewport.draw();
        glfwPollEvents();
    }

    return 0;
}
