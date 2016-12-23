// -*- C++ -*-
// geometry_object.hpp --
//

#pragma once

#include <core/geo/sphere.hpp>
#include <core/geo/plane.hpp>
#include <core/geo/disk.hpp>

#include <vector>


namespace yapt
{
    class GeometryPull
    {
	std::vector<Sphere> sphere;
	std::vector<Plane> plane;
	std::vector<Disk> disk;
    public:
	GeometryPull();

	void
	add_sphere(const Sphere & obj) noexcept;

	void
	add_plane(const Plane & obj) noexcept;

	void
	add_disk(const Disk & obj) noexcept;
    };

}
