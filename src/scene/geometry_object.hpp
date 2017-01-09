// -*- C++ -*-
// geometry_object.hpp --
//

#pragma once

#include <core/math/transforms.hpp>
#include <core/geo/primitive.hpp>
#include "material.hpp"

#include <vector>


namespace yapt
{
    class GeometryObject
    {
	Transform _transform;
	const Material * _material = nullptr;

	std::vector<Primitive*> _geo;
    public:
	GeometryObject();

	GeometryObject(const Transform & transform,
		       const Material & material);

	void
	add_primitive(Primitive * prim) noexcept;

	const std::vector<Primitive*> &
	get_primitives() const noexcept;

	const Transform &
	get_transform() const noexcept;

	const Material &
	get_material() const noexcept;
    };

}
