// -*- C++ -*-
// geometry_object.hpp --
//

#pragma once

#include <primitives/generic.hpp>

#include <vector>

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
    traverse(const Ray& ray, const std::vector<GenericPrimitive>& prims)
    {
        auto iter = prims.cbegin();
        auto isect = intersect(ray, *(iter++));
        for(; iter != prims.cend(); ++iter)
            isect = std::min(isect, intersect(ray, *iter));
        return isect;
    }
}
