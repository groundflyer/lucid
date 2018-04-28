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
    template <template <typename, size_t> typename AABBContainer,
              template <typename, size_t> typename SphereContainer,
              template <typename, size_t> typename TriangleContainer,
              template <typename, size_t> typename DiskPointContainer,
              template <typename, size_t> typename DiskNormalContainer,
              template <typename, size_t> typename PlanePointContainer,
              template <typename, size_t> typename PlaneNormalContainer>
    using GenericPrimitive_ = std::variant<AABB_<AABBContainer>,
                                           Sphere_<SphereContainer>,
                                           Triangle_<TriangleContainer>,
                                           Disk_<DiskPointContainer, DiskNormalContainer>,
                                           Plane_<PlanePointContainer, PlaneNormalContainer>>;

    using GenericPrimitive = std::variant<AABB, Sphere, Triangle, Disk, Plane>;


    template <template <typename, size_t> typename AABBContainer,
              template <typename, size_t> typename SphereContainer,
              template <typename, size_t> typename TriangleContainer,
              template <typename, size_t> typename DiskPointContainer,
              template <typename, size_t> typename DiskNormalContainer,
              template <typename, size_t> typename PlanePointContainer,
              template <typename, size_t> typename PlaneNormalContainer,
              template <typename, size_t> typename RayPContainer,
			  template <typename, size_t> typename RayNContainer>
    constexpr auto
    intersect(const Ray_<RayPContainer, RayNContainer>& ray,
              const GenericPrimitive_<AABBContainer, SphereContainer, TriangleContainer,
              DiskPointContainer, DiskNormalContainer,
              PlanePointContainer, PlaneNormalContainer>& prim,
              const Range<real>& range = Range<real>())
    {
        return std::visit([&](const auto& held_prim) { return intersect(ray, held_prim, range); }, prim);
    }
}
