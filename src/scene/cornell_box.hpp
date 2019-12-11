// -*- C++ -*-
// cornell_box.hpp
//

#pragma once

#include <base/types.hpp>
#include <cameras/perspective.hpp>
#include <ray_traversal/primitives/generic.hpp>

namespace lucid
{
constexpr std::array<Point, 8> box_points{Point(-1, -1, 1),
                                          Point(-1, -1, -1),
                                          Point(1, -1, -1),
                                          Point(1, -1, 1),
                                          Point(-1, 1, 1),
                                          Point(-1, 1, -1),
                                          Point(1, 1, -1),
                                          Point(1, 1, 1)};

struct Material
{
    RGB diffuse{0, 0, 0};
    RGB emission{0, 0, 0};
};

struct MatGetter
{
    Material
    operator()(const std::size_t pid) const noexcept
    {
        static constexpr std::array<std::size_t, 7> mat_idxs{1, // white
                                                             2, // red
                                                             3, // green
                                                             1, // white
                                                             1, // white
                                                             4,
                                                             5};

        const static Material materials[]{
            Material{RGB{0}, RGB{0}},                   // black default
            Material{RGB{1}, RGB{0}},                   // white
            Material{RGB{1, 0.1_r, 0.1_r}, RGB{0}},     // red
            Material{RGB{0.1_r, 1, 0.1_r}, RGB{0}},     // green
            Material{RGB{0.1_r, 0.5_r, 0.7_r}, RGB{0}}, // sphere
            Material{RGB{0}, RGB{10}}                   // light
        };
        return materials[mat_idxs[pid]];
    }
};

constexpr std::tuple box_geo_descr{
    std::index_sequence<0, 3, 2, 1>{}, // floor
    std::index_sequence<0, 1, 5, 4>{}, // left wall
    std::index_sequence<2, 3, 7, 6>{}, // right wall
    std::index_sequence<4, 5, 6, 7>{}, // ceiling
    std::index_sequence<1, 2, 6, 5>{}  // back wall
};

// Cornell Box Primitives
using QuadRef     = Quad_<StaticSpan>;
using CBPrimTypes = typelist<Sphere, QuadRef, Disk>;
using CBPrim      = typename CBPrimTypes::variant;
using ObjectData  = std::pair<CBPrim, std::size_t>;

template <std::size_t... Pns>
constexpr QuadRef
make_wall_geo(std::index_sequence<Pns...>) noexcept
{
    return QuadRef{ref(std::get<Pns>(box_points))...};
}

constexpr auto
make_room() noexcept
{
    return std::tuple_cat(
        std::apply([](auto... pns) { return std::tuple{make_wall_geo(pns)...}; }, box_geo_descr),
        std::tuple{Sphere(Point(0.5_r, -0.6_r, 0.2_r), 0.4_r),
                   Disk(Point(0, 0.99_r, 0), Normal(0, -1, 0), 0.3_r)});
}

constexpr perspective::shoot
make_cam(const real& ratio) noexcept
{
    return perspective::shoot(
        radians(60_r), ratio, look_at(Point(0, 0, 4), Point(0, 0, 0), Normal(0, 1, 0)));
}

constexpr MatGetter
get_mat_getter() noexcept
{
    return MatGetter{};
}

} // namespace lucid
