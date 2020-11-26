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

using Image    = ScanlineImage<float, 4>;
using FilmRGBA = Film<Image>;

struct to_unsigned
{
    unsigned
    operator()(const std::string_view word) const noexcept
    {
        return std::atoi(word.data());
    }
};

constexpr std::tuple options{
    option<'r', 2>(to_unsigned{}, {640, 640}, "resolution", "Image resolution.", {"W", "H"}),
    option<'d'>(to_unsigned{}, 4, "depth", "Maximum bounces.", "N"),
    option<'q'>(to_unsigned{}, 100000, "queue-size", "Task dispatcher queue size.", "Q"),
    option<'f'>([](const std::string_view word) noexcept { return std::atof(word.data()); },
                2_r,
                "filter-width",
                "Pixel filter width.",
                "W")};

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
    const constexpr auto     room_geo   = CornellBox::geometry();
    const constexpr auto     mat_getter = CornellBox::mat_getter();

    const real bias = 0.001_r;

    using PathTracer = PathTracer_<std::default_random_engine,
                                   std::decay_t<decltype(room_geo)>,
                                   std::decay_t<decltype(mat_getter)>>;

    try
    {
        FilmRGBA   film{res};
        auto       da         = [](auto&&...) noexcept {};
        auto       viewport   = make_viewport(res, da, da, da);
        const real filter_rad = film._pixel_radius * filter_width;

        viewport.load_img(film.img);
        viewport.check_errors();
        logger.debug("OpenGL initialized");

        Dispatcher<PathTracer> dispatcher(qsize);
        const PixelUpdate      updater{TriangleFilter(filter_rad)};

        auto it = film.img.begin();

        std::size_t    avg_mrps = 0ul; // average ray per second
        std::size_t    mnumber  = 0ul; // number of measurements
        ElapsedTimer<> timer{};

        while(viewport.active())
        {
            std::size_t ray_count = 0ul;

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
                const auto& [depth, sample] = ret.value();
                film                        = sample_based_singular_update(film, updater, sample);
                ray_count += depth;
            }

            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(timer.restart()).count();
            const auto mrps = ray_count / elapsed; // MRay/s
            avg_mrps += mrps;
            ++mnumber;
            viewport.reload_img(film.img);
            viewport.draw();
            glfwPollEvents();
            logger.debug(Logger::flush, "{} MRay/s", mrps);
        }

        logger.debug("Average MRay/s: {}", avg_mrps / mnumber);
    }
    catch(const GLenum& er)
    {
        logger.critical("OpenGL error: {}", er);
    }
    catch(const int& er)
    {
        logger.critical("GLFW init error: {}", er);
    }

    logger.debug("Good bye.");

    return 0;
}
