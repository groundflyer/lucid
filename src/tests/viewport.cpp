// -*- C++ -*-
// viewport.cpp
#include <gui/viewport.hpp>
#include <gui/observer.hpp>
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


struct MouseAction
{
    Logger* logger = nullptr;
    Vec2 prev_pos{0_r};
    bool pressed = false;

    void
    operator()(int button, bool _pressed, int /* mods */) noexcept
    {
        pressed = (button == GLFW_MOUSE_BUTTON_LEFT) && _pressed;
    }

    void
    operator()(const Vec2& new_pos) noexcept
    {
        if (pressed)
        {
            Vec2 delta = new_pos - prev_pos;
            prev_pos = new_pos;
            logger->debug("Movement delta = {}", delta.data());
        }
    }
};

using Viewport = _Viewport<MouseAction>;
using FilmRGBA = Film<ScanlineImage<float, 4>>;

int
main()
{
    Logger logger(Logger::DEBUG);

    Viewport::init(Vec2u(640, 480), MouseAction{&logger});

    FilmRGBA film{Vec2u(Viewport::get_res())};

    const auto               room_geo   = CornellBox::geometry();
    const auto               mat_getter = CornellBox::mat_getter();
    using Simple = Constant<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;
    auto render = [&](const perspective::shoot& cam)
                  {
                      for(auto it = film.img.begin(); it != film.img.end(); ++it)
                      {
                          const Vec2 pp = film.sample_space(it.pos());
                          Simple     ig{cam(pp), &room_geo, &mat_getter, pp};
                          *it = RGBA(ig().second);
                      }
                      Viewport::load_img(film.img);
                  };

    auto cam = Observable(CornellBox::camera(), render);

    render(cam.value());

    while(Viewport::active())
    {
        Viewport::draw();
        glfwPollEvents();
    }

    Viewport::cleanup();

    return 0;
}
