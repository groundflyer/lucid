// -*- C++ -*-
// image.hpp
//

#pragma once

#include "image.hpp"

#include <string>

namespace lucid
{
    void
    write_ppm(const Image<unsigned char, 3>&,
              const std::string& filename);
}
