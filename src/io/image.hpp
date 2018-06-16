// -*- C++ -*-
// image.hpp
//

#pragma once

#include <image/image.hpp>

#include <string>

namespace yapt
{
    void
    write_ppm(const Image<float, 3>&,
              const std::string& filename);
}
