// -*- C++ -*-
// cornell_box_mc.cpp
#include "benchmark.hpp"

#include <image/io.hpp>
#include <image_reconstruction/film.hpp>
#include <integrators/basic.hpp>
#include <sampling/film.hpp>
#include <scene/cornell_box.hpp>
#include <utils/argparse.hpp>
#include <utils/dispatcher.hpp>
#include <utils/logging.hpp>

#include <fmt/format.h>

using namespace lucid;
using namespace argparse;
using namespace std::literals;
using namespace fmt::literals;

static constexpr auto                          room_geo   = CornellBox::geometry();
static constexpr auto                          mat_getter = CornellBox::mat_getter();
static thread_local std::random_device         rd;
static thread_local std::default_random_engine g(rd());

using PathTracer = PathTracer_<std::default_random_engine,
                               std::decay_t<decltype(room_geo)>,
                               std::decay_t<decltype(mat_getter)>>;
using Image      = ScanlineImage<float, 4>;
using FilmRGBA   = Film<Image>;

struct to_unsigned
{
    unsigned
    operator()(const std::string_view word) const noexcept
    {
        return std::atoi(word.data());
    }
};

constexpr std::tuple options{
    option<'q'>(to_unsigned{}, 100000, "queue-size", "Task dispatcher queue size.", "Q"),
    option<'t'>(to_unsigned{}, 8, "threads", "Number of threads", "T"),
    option<'l'>(identity, "/dev/null", "log", "File to log to", "FILE")};

int
main(int argc, char* argv[])
{
    ArgsRange  args(argc, argv);
    const auto parse_results = parse(options, args, StandardErrorHandler(args, options));
    const constexpr unsigned     nsamples     = 64;
    const constexpr std::uint8_t max_depth    = 4;
    const constexpr real         filter_width = 2;
    const constexpr Vec2u        res(640, 640);
    const unsigned               qsize   = parse_results.get_opt<'q'>();
    const unsigned               threads = parse_results.get_opt<'t'>();
    LogFile                      log(parse_results.get_opt<'l'>());
    Logger                       logger(Logger::DEBUG);

    const perspective::shoot cam       = CornellBox::camera();
    const real               bias      = 0.001_r;
    std::size_t              ray_count = 0ul;

    FilmRGBA          film{res};
    const PixelUpdate updater{TriangleFilter(filter_width)};

    Dispatcher<PathTracer> dispatcher(qsize, threads);

    constexpr float num_pix = product(res);

    auto render = [&]() noexcept {
        std::size_t pos = 1ul;
        for(auto it = film.img.begin(); it != film.img.end(); ++it, ++pos)
        {
            for(unsigned i = 0u; i < nsamples; ++i)
            {
                const Vec2 sample_pos =
                    sample_pixel(g, film._pixel_width, film.sample_space(it.pos()));
                if(!dispatcher.try_submit(PathTracer{
                       &g, &room_geo, &mat_getter, cam(sample_pos), max_depth, bias, sample_pos}))
                    while(const auto ret = dispatcher.fetch_result<PathTracer>())
                    {
                        const auto& [depth, sample] = ret.value();
                        film = sample_based_singular_update(film, updater, sample);
                        ray_count += depth;
                    }
            }
            logger.info(Logger::flush, "{:05.1f}%", (static_cast<float>(pos) / num_pix) * 100.f);
        }
    };

    auto       ns   = time_it(render);
    const auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    const auto ss   = std::chrono::duration_cast<std::chrono::seconds>(ns);
    const auto mrps = ray_count / ms.count();

    write_ppm(film.img, "cornell_box_mc_{}spp.ppm"_format(nsamples));
    fmt::print("Cornell Box Monte-Carlo: {} samples: time {}, {} MRay/s\n", nsamples, ss, mrps);
    log.append("{},{:%Y%m%d},{},{},{}\n",
               repo_hash,
               fmt::localtime(std::time(nullptr)),
               threads,
               qsize,
               mrps);

    return 0;
}
