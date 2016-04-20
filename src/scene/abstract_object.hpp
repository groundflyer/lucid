// -*- C++ -*-
// object.hpp --
//

#pragma once

#include <core/math/transforms.hpp>
#include <core/geo/intersection.hpp>


namespace yapt
{
    class AbstractObject
    {
    public:
	AbstractObject() {};

	virtual void
	set_transform(const Trasform & transform) noexcept = 0;
    };
}
