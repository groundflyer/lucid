// -*- C++ -*-
// geometry_object.cpp --

#include "geometry_object.hpp"


namespace yapt
{
    GeometryObject::GeometryObject() {}

    GeometryObject::GeometryObject(const Transform & transform, const Material& material) : _transform(transform), _material(&material) { }

    void
    GeometryObject::add_primitive(Primitive * prim) noexcept
    {
	if (prim)
	    _geo.push_back(prim);
    }

    const std::vector<Primitive*> &
    GeometryObject::get_primitives() const noexcept
    {
	return _geo;
    }

    const Transform &
    GeometryObject::get_transform() const noexcept
    {
	return _transform;
    }

    const Material &
    GeometryObject::get_material() const noexcept
    {
	return *_material;
    }
}
