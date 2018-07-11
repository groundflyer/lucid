// -*- C++ -*-
// image.cpp

#include "image.hpp"

#include <fstream>
#include <algorithm>

void
yapt::write_ppm(const yapt::Image<float, 3>& img,
                const std::string& filename)
{
    std::ofstream out(filename);
    out << "P3\n"
        << img.res() << '\n'
        << 255 << '\n';

    for (auto color : img)
        out << transform(Vector<int, 3, std::array>(RGB(color) * 255),
                         [](const int& val)
                         { return std::clamp(val, 0, 255); }) << ' ';
}
