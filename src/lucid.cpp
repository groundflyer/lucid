// -*- C++ -*-
// lucid.cpp

#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>
#include <utils/dispatcher.hpp>
#include <utils/logging.hpp>
#include <utils/timer.hpp>
#include <utils/tuple.hpp>

#include <string>

using namespace lucid;
using namespace std::literals;

static std::random_device                      rd;
static thread_local std::default_random_engine g(rd());

int
main(int argc, char* argv[])
{
    const std::uint8_t max_depth = argc > 1 ? std::stoi(argv[1]) : 4;

    const Vec2u res(640, 640);
    const auto& [w, h] = res;
    const real ratio   = static_cast<real>(w) / static_cast<real>(h);

    Logger logger(Logger::DEBUG);

    const perspective::shoot cam        = CornellBox::camera(ratio);
    const auto               room_geo   = CornellBox::geometry();
    const auto               mat_getter = CornellBox::mat_getter();

    const real bias = 0.001_r;

    using PathTracer = PathTracer_<std::default_random_engine,
                                   std::decay_t<decltype(room_geo)>,
                                   std::decay_t<decltype(mat_getter)>>;
    using Simple = Constant<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;

    try
    {
        Viewport::init(w, h);
        Film<ScanlineImage<float, 4>> film(Viewport::get_res());
        const real                    filter_rad = film.pixel_radius() * 4_r;

        Viewport::load_img(film.img);
        Viewport::check_errors();
        logger.debug("OpenGL initialized");

        std::atomic_bool               produce{true};
        Dispatcher<PathTracer, Simple> dispatcher;
        const auto                     producer = [&]() {
            while(produce.load(std::memory_order_relaxed))
                for(auto it = film.img.begin();
                    it != film.img.end() && produce.load(std::memory_order_relaxed);
                    ++it)
                {
                    const Vec2 pixel_pos = film.device_coords(it.pos());
                    const Vec2 sample_pos = sample_pixel(g, film.pixel_size, pixel_pos);
                    // dispatcher.try_submit(
                    //     Simple{cam(sample_pos), &room_geo, &mat_getter, sample_pos});
                    dispatcher.try_submit(PathTracer{
                        &g, &room_geo, &mat_getter, cam(sample_pos), max_depth, bias, sample_pos});
                }
        };
        std::thread producer_thread(producer);

        const PixelUpdate updater{TriangleFilter(filter_rad)};

        while(Viewport::active())
        {
            while(const auto ret = dispatcher.fetch_result<PathTracer>())
            {
                const Sample& sample = ret.value();
                update_pixels(film, updater, sample);
            }

            Viewport::reload_img(film.img);
            Viewport::draw();
            glfwPollEvents();
        }

        produce.store(false, std::memory_order_relaxed);
        producer_thread.join();
    }
    catch(const GLenum& er)
    {
        logger.critical("OpenGL error: {}", er);
    }
    catch(const int& er)
    {
        logger.critical("GLFW init error: {}", er);
    }

    Viewport::cleanup();
    logger.debug("Good bye.");

    return 0;
}
