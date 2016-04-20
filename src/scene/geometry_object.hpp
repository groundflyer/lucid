// -*- C++ -*-
// geometry_object.hpp --
//

#pragma once

#include "abstract_object.hpp"


namespace yapt
{
    class GeometryObject : public AbstractObject
    {
	Trasform _transform;

    public:
	GeometryObject();

	void
	set_transform(const Trasform & transform);
    };

}
