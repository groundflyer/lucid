// -*- C++ -*-
// lucid.cpp

#include <base/rng.hpp>
#include <cameras/perspective.hpp>
#include <gui/viewport.hpp>
#include <ray_traversal/primitives/generic.hpp>
#include <utils/dispatcher.hpp>
#include <utils/logging.hpp>
#include <utils/timer.hpp>
#include <utils/tuple.hpp>

#include <string>

using namespace lucid;
using namespace std::literals;

// convert image coordinates to device coordinates
constexpr Vec2
to_device_coords(const Vec2& pos, const Vec2& res) noexcept
{
    return (pos - res * 0.5_r) / res;
}

constexpr Vec2
to_device_coords(const Vec2u& pos, const Vec2u& res) noexcept
{
    return to_device_coords(Vec2(pos), Vec2(res));
}

Vec3
sample_hemisphere(const Normal& n, const Vec2& u) noexcept
{
    const auto& [u1, u2] = u;
    const auto r         = 2_r * Pi * u2;
    const auto phi       = math::sqrt(1_r - pow<2>(u1));
    return basis_matrix(n).dot(Vec3(math::cos(r) * phi, math::sin(r) * phi, u1));
}

static const constexpr std::array<Point, 8> box_points{
    Point(-1, -1, 1), Point(-1, -1, -1), Point(1, -1, -1), Point(1, -1, 1),
    Point(-1, 1, 1),  Point(-1, 1, -1),  Point(1, 1, -1),  Point(1, 1, 1)};

struct Material
{
    RGB diffuse{0, 0, 0};
    RGB emission{0, 0, 0};
};

static const constexpr Material materials[]{
    Material{RGB{0}, RGB{0}},                   // black default
    Material{RGB{1}, RGB{0}},                   // white
    Material{RGB{1, 0.1_r, 0.1_r}, RGB{0}},     // red
    Material{RGB{0.1_r, 1, 0.1_r}, RGB{0}},     // green
    Material{RGB{0.1_r, 0.5_r, 0.7_r}, RGB{0}}, // sphere
    Material{RGB{0}, RGB{10}}                   // light
};

static const constexpr std::tuple box_geo_descr{
    std::index_sequence<0, 3, 2, 1>{}, // floor
    std::index_sequence<0, 1, 5, 4>{}, // left wall
    std::index_sequence<2, 3, 7, 6>{}, // right wall
    std::index_sequence<4, 5, 6, 7>{}, // ceiling
    std::index_sequence<1, 2, 6, 5>{}  // back wall
};

static const constexpr std::array<std::size_t, 5> box_mat_idxs{
      1, // white
      2, // red
      3, // green
      1, // white
      1  // white
};

// Cornell Box Primitives
using QuadRef = Quad_<StaticSpan>;
using CBPrimTypes = typelist<Sphere, QuadRef, Disk>;
using CBPrim = typename CBPrimTypes::variant;
using ObjectData = std::pair<CBPrim, std::size_t>;

template <std::size_t... Pns>
constexpr QuadRef
make_wall_geo(std::index_sequence<Pns...>) noexcept
{
    return QuadRef{ref(std::get<Pns>(box_points))...};
}

constexpr auto
make_room() noexcept
{
    return std::apply([](auto... pns) { return std::tuple{make_wall_geo(pns)...}; }, box_geo_descr);
}

static std::random_device                      rd;
static thread_local std::default_random_engine g(rd());

template <typename Scene, typename MaterialGetter>
struct PathTracer_
{
    Ray                       ray;
    Scene const*              scene;
    MaterialGetter const*     material_getter;
    std::uint8_t              max_depth;
    real                      bias;
    Vec2                      ndc;
    Vec2                      sample_pos;
    Image<float, 3>::iterator it;

    auto
    operator()() noexcept
    {
        RGB  radiance{1};
        bool has_rad = false;

        for(std::size_t depth = 0; depth < max_depth; ++depth)
        {
            const auto& [ro, rd]    = ray;
            const auto [isect, pid] = hider(ray, *scene);

            if(!isect) break;

            const auto& [color, emission] = (*material_getter)(pid);
            has_rad |= any(emission > 0_r);

            if(all(almost_equal(color, 0_r, 10)))
            {
                radiance *= emission;
                break;
            }

            const Normal n = lucid::visit(
                pid,
                [&, &iss = isect](const auto& prim) { return normal(ray, iss, prim); },
                *scene);

            const Point  p = ro + rd * isect.t;
            const Normal new_dir(sample_hemisphere(n, Vec2(rand<real, 2>(g))));

            radiance = radiance * color * std::max(dot(n, new_dir), 0_r) + emission;

            ray = Ray(p + new_dir * bias, new_dir);
        }

        return std::tuple{radiance * has_rad, ndc, sample_pos, it};
    }
};

using Sample = std::pair<Vec2, RGB>;

template <typename Generator>
Vec2
sample_pixel(Generator& g, const real size, const Vec2& ndc) noexcept
{
    Vec2 s{rand<real, 2>(g)};
    s *= size;
    return ndc + s;
}

template <template <typename, std::size_t> typename Container>
RGB
update_pixel(const RGB_<Container>& color,
             const real             size,
             const Vec2&            ndc,
             const Sample&          sample) noexcept
{
    const auto& [sndc, sval] = sample;
    real dist                = distance(ndc, sndc);
    dist /= size;
    const real weight = 1_r - dist;
    return (color + sval * weight) / (1_r + weight);
}

RGB
reset_pixel(const real size, const Vec2& ndc, const Sample& sample) noexcept
{
    const auto& [sndc, sval] = sample;
    real dist                = distance(ndc, sndc);
    dist /= size;
    const real weight = 1_r - dist;
    return sval * weight;
}

int
main(int argc, char* argv[])
{
    const std::uint8_t max_depth = argc > 1 ? std::stoi(argv[1]) : 4;

    const auto& [w, h]       = vp::res;
    const real         ratio = static_cast<real>(w) / static_cast<real>(h);
    perspective::shoot cam(
        radians(60_r), ratio, look_at(Point(0, 0, 4), Point(0, 0, 0), Normal(0, 1, 0)));

    Logger logger(Logger::DEBUG);

    try
    {
        vp::init();
    }
    catch(const std::runtime_error& ex)
    {
        logger.critical("OpenGL Error during initialization: {}", ex.what());
        return 1;
    }

    const auto room_geo = tuple_cat(make_room(),
                                    std::tuple{Sphere(Point(0.5_r, -0.6_r, 0.2_r), 0.4_r),
                                               Disk(Point(0, 0.99_r, 0), Normal(0, -1, 0), 0.3_r)});

    const auto room_mat_ids = array_cat(box_mat_idxs, std::array<std::size_t, 2>{4, 5});

    const auto mat_getter = [&](const std::size_t pid) { return materials[room_mat_ids[pid]]; };

    const real bias = 0.001_r;

    using PathTracer =
        PathTracer_<std::decay_t<decltype(room_geo)>, std::decay_t<decltype(mat_getter)>>;

    try
    {
        Dispatcher<PathTracer> dispatcher;

        Image<float, 3> img(vp::res);

        const real pixel_size = 1_r / lucid::max(vp::res);

        std::atomic_bool produce{true};

        const auto producer = [&]() {
            while(produce.load(std::memory_order_relaxed))
                for(auto it = img.begin();
                    it != img.end() && produce.load(std::memory_order_relaxed);
                    ++it)
                {
                    const Vec2 ndc  = to_device_coords(it.pos(), vp::res);
                    const Vec2 spos = sample_pixel(g, pixel_size, ndc);
                    dispatcher.try_submit(PathTracer{
                        cam(spos), &room_geo, &mat_getter, max_depth, bias, ndc, spos, it});
                }
        };

        vp::load_img(img);

        vp::check_errors();

        ElapsedTimer<> timer;

        for(auto it = img.begin(); it != img.end(); ++it)
        {
            const Vec2 ndc  = to_device_coords(it.pos(), vp::res);
            const Vec2 spos = sample_pixel(g, pixel_size, ndc);

            PathTracer path_tracer{
                cam(spos), &room_geo, &mat_getter, max_depth, bias, ndc, spos, it};
            const auto ret  = path_tracer();
            const RGB  sval = std::get<0>(ret);

            *it = reset_pixel(pixel_size, ndc, Sample{spos, sval});
        }

        std::thread producer_thread(producer);

        vp::reload_img(img);
        vp::draw();
        glfwPollEvents();

        std::size_t spp = 1;

        while(vp::active())
        {
            while(const auto ret = dispatcher.fetch_result<PathTracer>())
            {
                const auto& [sval, ndc, spos, it] = ret.value();
                *it = update_pixel(*it, pixel_size, ndc, Sample{spos, sval});
                ++spp;
            }

            vp::reload_img(img);
            vp::draw();
            glfwPollEvents();
        }

        logger.info("Rendering time {:12%H:%M:%S}", timer.elapsed());
        logger.info("Total pixel samples: {}", spp);
        produce.store(false, std::memory_order_relaxed);
        producer_thread.join();
    }
    catch(GLenum er)
    {
        logger.critical("OpenGL error: {}", er);
    }

    vp::cleanup();

    return 0;
}
