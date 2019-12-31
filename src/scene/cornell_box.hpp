// -*- C++ -*-
// cornell_box.hpp
//

#pragma once

#include <base/types.hpp>
#include <cameras/perspective.hpp>
#include <ray_traversal/primitives/generic.hpp>

namespace lucid
{

class CornellBox
{
    static const constexpr std::array<Point, 8> box_points{Point(-1, -1, 1),
                                                           Point(-1, -1, -1),
                                                           Point(1, -1, -1),
                                                           Point(1, -1, 1),
                                                           Point(-1, 1, 1),
                                                           Point(-1, 1, -1),
                                                           Point(1, 1, -1),
                                                           Point(1, 1, 1)};

    static const constexpr std::tuple box_geo_descr{
        std::index_sequence<0, 3, 2, 1>{}, // floor
        std::index_sequence<0, 1, 5, 4>{}, // left wall
        std::index_sequence<2, 3, 7, 6>{}, // right wall
        std::index_sequence<4, 5, 6, 7>{}, // ceiling
        std::index_sequence<1, 2, 6, 5>{}  // back wall
    };

    using QuadRef = Quad_<StaticSpan>;

    template <std::size_t... Pns>
    static constexpr QuadRef
    make_wall_geo(std::index_sequence<Pns...>) noexcept
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

    struct Material
    {
        RGB diffuse{0, 0, 0};
        RGB emission{0, 0, 0};
    };

    class MatGetter
    {
        static const constexpr std::array<std::size_t, 7> mat_idxs{1, // white
                                                                   2, // red
                                                                   3, // green
                                                                   1, // white
                                                                   1, // white
                                                                   4,
                                                                   5};

        static const constexpr RGB      black{0_r};
        static const constexpr RGB      white{1_r};
        static const constexpr RGB      red{1, 0.1_r, 0.1_r};
        static const constexpr RGB      green{0.1_r, 1, 0.1_r};
        static const constexpr Material materials[]{Material{black, black},
                                                    Material{white, black},
                                                    Material{red, black},
                                                    Material{green, black},
                                                    Material{green, black},
                                                    Material{black, RGB(10_r)}};

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
            std::tuple{Sphere(Point(0.5_r, -0.6_r, 0.2_r), 0.4_r),
                       Disk(Point(0, 0.99_r, 0), Normal(0, -1, 0), 0.3_r)});
    }

    static constexpr perspective::shoot
    camera(const real& ratio) noexcept
    {
        return perspective::shoot(
            radians(60_r), ratio, look_at(Point(0, 0, 4), Point(0, 0, 0), Normal(0, 1, 0)));
    }

    static constexpr MatGetter
    mat_getter() noexcept
    {
        return MatGetter{};
    }
};
} // namespace lucid
