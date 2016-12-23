// -*- C++ -*-

#include "geometry_object.hpp"


namespace yapt
{
    GeometryObject::GeometryObject() {}

    void
    GeometryObject::add_primitive(Primitive * prim) noexcept
    {
	if (prim)
	    _geo.push_back(prim);
    }

    void
    GeometryObject::set_material(const Material & material) noexcept
    { _material = material; }

    void
    GeometryObject::set_transform(const Trasform & transform) noexcept
    { _transform = transform; }
}
