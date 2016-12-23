// -*- C++ -*-
// image.h --
//

#pragma once

#include <string>
#include "film.hpp"


namespace yapt
{
    bool
    write_image(const Film & film,
		const std::string & filename) noexcept;
}
