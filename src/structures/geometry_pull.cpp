// -*- C++ -*-

#include "geometry_object.h"


namespace yapt
{
    GeometryPull::GeometryPull() {}

    void
    GeometryPull::add_triangle(const Triangle & obj) noexcept
    { triangle.push_back(obj); }

    void
    GeometryPull::add_sphere(const Sphere & obj) noexcept
    { sphere.push_back(obj); }

    void
    GeometryPull::add_plane(const Plane & obj) noexcept
    { plane.push_back(obj); }

    void
    GeometryPull::add_disk(const Disk & obj) noexcept
    { disk.push_back(obj); }

    void
    GeometryPull::set_material(const Material & material) noexcept
    { _material = material; }

    void
    GeometryPull::set_transform(const Trasform & transform) noexcept
    { _transform = transform; }
}
