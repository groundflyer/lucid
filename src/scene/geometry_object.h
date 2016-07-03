// -*- C++ -*-
// geometry_object.hpp --
//

#pragma once

#include "abstract_object.hpp"
#include "material.hpp"
#include <core/geo/primitive.hpp>

#include <vector>


namespace yapt
{
    class GeometryObject : public AbstractObject
    {
	Trasform _transform;
	Material _material;

	std::vector<Primitive*> _geo;
    public:
	GeometryObject();

	void
	set_material(const Material & material) noexcept;

	void
	set_transform(const Trasform & transform) noexcept;
    };

}
