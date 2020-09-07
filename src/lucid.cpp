// -*- C++ -*-
// lucid.cpp

#include <gui/viewport.hpp>
#include <image_reconstruction/film.hpp>
#include <image_reconstruction/filtering.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>
#include <utils/argparse.hpp>
#include <utils/dispatcher.hpp>
#include <utils/logging.hpp>
#include <utils/timer.hpp>
#include <utils/tuple.hpp>

using namespace lucid;
using namespace std::literals;
using namespace argparse;

struct to_unsigned
{
    unsigned
    operator()(const std::string_view word) const noexcept
    {
        return std::atoi(word.data());
    }
};

struct to_real
{
    real
    operator()(const std::string_view word) const noexcept
    {
        return std::atof(word.data());
    }
};

struct dummy_action
{
    template <typename... Args>
    void
    operator()(Args&&...) const noexcept
    {
    }
};

using Viewport = _Viewport<dummy_action>;

constexpr std::tuple options{
    option<'r', 2>(to_unsigned{}, {640, 640}, "resolution", "Image resolution.", {"W", "H"}),
    option<'d'>(to_unsigned{}, 4, "depth", "Maximum bounces.", "N"),
    option<'q'>(to_unsigned{}, 100000, "queue-size", "Task dispatcher queue size.", "Q"),
    option<'f'>(to_real{}, 2_r, "filter-width", "Pixel filter width.", "W")};

static_assert(!keywords_have_space(options));

static std::random_device                      rd;
static thread_local std::default_random_engine g(rd());

int
main(int argc, char* argv[])
{
    ArgsRange  args(argc, argv);
    const auto parse_results = parse(options, args, StandardErrorHandler(args, options));

    const std::uint8_t max_depth    = parse_results.get_opt<'d'>();
    const unsigned     qsize        = parse_results.get_opt<'q'>();
    const real         filter_width = parse_results.get_opt<'f'>();

    const Vec2u res(parse_results.get_opt<'r'>());

    Logger logger(Logger::DEBUG);

    const perspective::shoot cam        = CornellBox::camera();
    const auto               room_geo   = CornellBox::geometry();
    const auto               mat_getter = CornellBox::mat_getter();

    const real bias = 0.001_r;

    using PathTracer = PathTracer_<std::default_random_engine,
                                   std::decay_t<decltype(room_geo)>,
                                   std::decay_t<decltype(mat_getter)>>;

    try
    {
        Viewport::init(res, dummy_action{});
        Film<ScanlineImage<float, 4>> film{Vec2u(Viewport::get_res())};
        const real                    filter_rad = film._pixel_radius * filter_width;

        Viewport::load_img(film.img);
        Viewport::check_errors();
        logger.debug("OpenGL initialized");

        Dispatcher<PathTracer> dispatcher(qsize);
        const PixelUpdate      updater{TriangleFilter(filter_rad)};

        auto it = film.img.begin();

        while(Viewport::active())
        {
            Vec2 sample_pos;
            do
            {
                if(it != film.img.end())
                    ++it;
                else
                    it = film.img.begin();
                sample_pos = sample_pixel(g, film._pixel_width, film.sample_space(it.pos()));
            } while(dispatcher.try_submit(PathTracer{
                &g, &room_geo, &mat_getter, cam(sample_pos), max_depth, bias, sample_pos}));

            while(const auto ret = dispatcher.fetch_result<PathTracer>())
            {
                const Sample& sample = ret.value();
                film                 = sample_based_singular_update(film, updater, sample);
            }

            Viewport::reload_img(film.img);
            Viewport::draw();
            glfwPollEvents();
        }
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
