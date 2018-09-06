// -*- C++ -*-
// image.cpp

#include "io.hpp"

#include <fstream>
#include <algorithm>

void
yapt::write_ppm(const yapt::Image<unsigned char, 3>& img,
                const std::string& filename)
{
    std::ofstream out(filename);
    out << "P3\n"
        << img.res() << '\n'
        << 255 << '\n';

    for (auto color : img)
        out << Vector<int, 3, std::array>(color) << ' ';
}
