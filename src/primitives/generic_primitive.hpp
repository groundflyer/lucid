// -*- C++ -*-
// primitive.hpp
//

#pragma once

#include "aabb.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "disk.hpp"
#include "plane.hpp"

#include <variant>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    using GenericPrimitive_ = std::variant<AABB_<Container>,
                                           Sphere_<Container>,
                                           Triangle_<Container>,
                                           Disk_<Container>,
                                           Plane_<Container>>;

    using GenericPrimitive = std::variant<AABB, Sphere, Triangle, Disk, Plane>;


    template <template <typename, size_t> typename PrimContainer,
			  template <typename, size_t> typename RayContainer>
    constexpr auto
    intersect(const Ray_<RayContainer>& ray,
              const GenericPrimitive_<PrimContainer>& prim,
              const Range<real>& range = Range<real>())
    {
        return std::visit([&](const auto& held_prim) { return intersect(ray, held_prim, range); }, prim);
    }
}
