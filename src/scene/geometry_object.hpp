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
	Material * _material = nullptr;

	std::vector<Primitive*> _geo;
    public:
	GeometryObject();

	GeometryObject(const Transform & transform,
		       const Material & material);

	void
	add_primitive(Primitive * prim) noexcept;
    };

}
