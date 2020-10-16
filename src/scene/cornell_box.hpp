// -*- C++ -*-
// cornell_box.hpp
//

#pragma once

#include <base/types.hpp>
#include <cameras/perspective.hpp>
#include <primitives/generic.hpp>
#include <sampling/common.hpp>

namespace lucid
{

class CornellBox
{
    static constexpr const std::array<Vec3, 8> box_points{Vec3(-1, -1, 1),
                                                          Vec3(-1, -1, -1),
                                                          Vec3(1, -1, -1),
                                                          Vec3(1, -1, 1),
                                                          Vec3(-1, 1, 1),
                                                          Vec3(-1, 1, -1),
                                                          Vec3(1, 1, -1),
                                                          Vec3(1, 1, 1)};

    static constexpr const std::tuple box_geo_descr{
        std::index_sequence<0, 3, 2, 1>{}, // floor
        std::index_sequence<0, 1, 5, 4>{}, // left wall
        std::index_sequence<2, 3, 7, 6>{}, // right wall
        std::index_sequence<4, 5, 6, 7>{}, // ceiling
        std::index_sequence<1, 2, 6, 5>{}  // back wall
    };

    using QuadRef = Quad_<StaticSpan>;

    template <std::size_t... Pns>
    static constexpr QuadRef make_wall_geo(std::index_sequence<Pns...>) noexcept
    {
        return QuadRef{ref(std::get<Pns>(box_points))...};
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

        static constexpr const std::array<std::size_t, 7> mat_idxs{1, // white
                                                                   2, // red
                                                                   3, // green
                                                                   1, // white
                                                                   1, // white
                                                                   4,
                                                                   5};

        static constexpr const RGB black{0_r};
        static constexpr const RGB white{1_r};
        static constexpr const RGB red{1, 0.1_r, 0.1_r};
        static constexpr const RGB green{0.1_r, 1, 0.1_r};
        using Material = std::pair<BSDF, Emission>;
        static constexpr const Material materials[]{{{black}, {black}},
                                                    {{white}, {black}},
                                                    {{red}, {black}},
                                                    {{green}, {black}},
                                                    {{green}, {black}},
                                                    {{black}, {RGB(10_r)}}};

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
        return std::tuple_cat(
            std::apply([](auto... pns) { return std::tuple{make_wall_geo(pns)...}; },
                       box_geo_descr),
            std::tuple{Sphere(Vec3(0.5_r, -0.6_r, 0.2_r), 0.4_r),
                       Disk(Vec3(0, 0.99_r, 0), Vec3(0, -1, 0), 0.3_r)});
    }

    static perspective::shoot
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
