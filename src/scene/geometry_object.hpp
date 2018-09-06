// -*- C++ -*-
// geometry_object.hpp --
//

#pragma once

#include <traversal/generic.hpp>

#include <vector>
#include <utility>

namespace yapt
{
    // class Material
    // {
    //     RGB m_color{1,1,1};

    // public:
    //     constexpr
    //     Material(const RGB& color) : m_color(color) {}

    //     constexpr auto
    //     eval(const Normal& n,
    //          const Normal& l) const noexcept
    //     { return m_color * n.dot(l); }
    // };

    struct Object
    {
        std::vector<GenericPrimitive> prims;
        // Mat4 transform;

        // constexpr
        // Object() : transform(Mat4::unit()) {}
    };

    auto
    tracked_intersect(const Ray& ray,
                      const GenericPrimitive& prim,
                      const Range<real>& range = Range<real>())
    { return std::pair(intersect(ray, prim, range), &prim); }

    auto
    traverse(const Ray& ray,
             const std::vector<GenericPrimitive>& prims,
             const Range<real>& range = Range<real>())
    {
        auto iter = prims.cbegin();
        auto isect = tracked_intersect(ray, *iter, range);
        for(++iter; iter != prims.cend(); ++iter)
            isect = std::min(isect, tracked_intersect(ray, *iter, range),
                             [](const auto& a, const auto& b)
                             { return a.first < b.first; });
        return isect;
    }

    auto
    occlusion(const Ray& ray,
              const std::vector<GenericPrimitive>& prims,
              const Range<real>& range = Range<real>())
    {
        auto iter = prims.cbegin();
        auto isect = intersect(ray, *iter, range);
        for(++iter; iter != prims.cend(); ++iter)
            isect = std::min(isect, intersect(ray, *iter, range));
        return bool(isect);
    }

    constexpr auto
    get_intersection_pos(const Ray& ray, const Intersection& isect)
    {
        const auto& [o, d] = ray;
        return o + d * isect.distance();
    }
}
