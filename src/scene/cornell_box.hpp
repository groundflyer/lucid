// -*- C++ -*-
// cornell_box.hpp
//

/// @file
/// Cornell Box scene description.

#pragma once

#include <base/types.hpp>
#include <cameras/perspective.hpp>
#include <primitives/primitives.hpp>
#include <sampling/common.hpp>

namespace lucid
{
class CornellBox
{
    // Box points coordinates
    static constexpr const std::array<Vec3, 8> room_points{Vec3(-1, -1, 1),  // 0
                                                           Vec3(-1, -1, -1), // 1
                                                           Vec3(1, -1, -1),  // 2
                                                           Vec3(1, -1, 1),   // 3
                                                           Vec3(-1, 1, 1),   // 4
                                                           Vec3(-1, 1, -1),  // 5
                                                           Vec3(1, 1, -1),   // 6
                                                           Vec3(1, 1, 1)};   // 7

    // Quad point indicies for room geometry
    static constexpr const std::tuple room_geo_descr{
        std::index_sequence<0, 3, 2, 1>{}, // floor
        std::index_sequence<0, 1, 5, 4>{}, // left wall
        std::index_sequence<2, 3, 7, 6>{}, // right wall
        std::index_sequence<4, 5, 6, 7>{}, // ceiling
        std::index_sequence<1, 2, 6, 5>{}  // back wall
    };

    // Quad point indicies for box geometry
    static constexpr const std::tuple box_geo_descr{
        std::index_sequence<1, 2, 3, 0>{},
        std::index_sequence<4, 5, 1, 0>{},
        std::index_sequence<6, 7, 3, 2>{},
        std::index_sequence<7, 6, 5, 4>{},
        std::index_sequence<5, 6, 2, 1>{},
        std::index_sequence<0, 4, 7, 3>{},
    };

    static constexpr const Vec3 box_scale{0.25_r, 0.5_r, 0.25_r};
    static constexpr const Vec3 box_pos{-0.5, get_y(box_scale) - 1_r, -0.25};
    static constexpr const Mat4 box_tr =
        rotate(radians(20_r), Vec3{0_r, 1_r, 0_r}) * translate(box_pos) * scale(box_scale);

    // Box point positions.
    static constexpr std::array<Vec3, 8> box_points = std::apply(
        [](const auto&... pns) noexcept {
            return std::array<Vec3, 8>{apply_transform_p(box_tr, pns)...};
        },
        room_points);

    static constexpr const Vec3 sphere_pos{0.5_r, -0.6_r, 0.2_r};
    static constexpr const real sphere_radius = 0.4_r;

    using QuadRef = Quad_<StaticSpan>;

    template <std::size_t N, std::size_t... Pns>
    static constexpr QuadRef
    make_wall_geo(const std::array<Vec3, N>& points, std::index_sequence<Pns...>) noexcept
    {
        return QuadRef{ref(std::get<Pns>(points))...};
    }

  public:
    CornellBox()                  = delete;
    CornellBox(const CornellBox&) = delete;
    CornellBox(CornellBox&&)      = delete;
    CornellBox&
    operator=(const CornellBox&) = delete;
    ~CornellBox()                = delete;

    class MatGetter
    {
        struct BSDF
        {
            struct Eval
            {
                const Vec3& n;
                const RGB&  color;

                RGB
                operator()(const Vec3& wi, const Vec3& /* wo */) const noexcept
                {
                    return color * std::max(dot(n, wi), 0_r);
                }
            };

            struct Sample
            {
                const Vec3& n;

                Vec3
                operator()(const Vec3& /* wo */, const Vec2& u) const noexcept
                {
                    return normalize(sample_hemisphere(n, u));
                }
            };

            const RGB color{0};

            constexpr std::pair<Eval, Sample>
            operator()(const Vec3& n) const noexcept
            {
                return {{n, color}, {n}};
            }
        };

        struct Emission
        {
            RGB color{0};

            constexpr RGB
            operator()() const noexcept
            {
                return color;
            }
        };

        // Material indicies for primitives
        static constexpr const std::array<std::size_t, 13> mat_idxs{   // room of 5 primitives
                                                                    1, // white floor
                                                                    2, // red wall
                                                                    3, // green wall
                                                                    1, // white ceiling
                                                                    1, // white wall
                                                                       // objects
                                                                    4, // green sphere
                                                                    5, // light
                                                                       // blue box of 6 primitives
                                                                    6,
                                                                    6,
                                                                    6,
                                                                    6,
                                                                    6,
                                                                    6};

        static constexpr const RGB black{0_r};
        static constexpr const RGB white{1_r};
        static constexpr const RGB red{1, 0.1_r, 0.1_r};
        static constexpr const RGB green{0.1_r, 1, 0.1_r};
        static constexpr const RGB blue{0.1_r, 0.1_r, 1_r};
        using Material = std::pair<BSDF, Emission>;
        static constexpr const Material materials[]{{{black}, {black}},     // 0
                                                    {{white}, {black}},     // 1
                                                    {{red}, {black}},       // 2
                                                    {{green}, {black}},     // 3
                                                    {{green}, {black}},     // 4
                                                    {{black}, {RGB(10_r)}}, // 5
                                                    {{blue}, {black}}};     // 6

      public:
        constexpr const Material&
        operator()(const std::size_t pid) const noexcept
        {
            return materials[mat_idxs[pid]];
        }
    };

    static constexpr auto
    geometry() noexcept
    {
        const constexpr real lx = 0.25_r;
        const constexpr real ly = 0.99_r;

        constexpr auto room = std::apply(
            [](const auto&... pns) noexcept {
                return std::tuple{make_wall_geo(room_points, pns)...};
            },
            room_geo_descr);

        constexpr auto box = std::apply(
            [](const auto&... pns) noexcept {
                return std::tuple{make_wall_geo(box_points, pns)...};
            },
            box_geo_descr);

        return std::tuple_cat(
            room,
            std::tuple{
                Sphere(sphere_pos, sphere_radius),
                // light
                Quad{Vec3{-lx, ly, -lx}, Vec3{lx, ly, -lx}, Vec3{lx, ly, lx}, Vec3{-lx, ly, lx}}},
            box);
    }

    static constexpr perspective::shoot
    camera(const real fov = radians(60_r)) noexcept
    {
        return perspective::shoot(convert_fov(fov),
                                  look_at(Vec3(0, 0, 4), Vec3(0, 0, 0), Vec3(0, 1, 0)));
    }

    static constexpr MatGetter
    mat_getter() noexcept
    {
        return MatGetter{};
    }
};
} // namespace lucid
