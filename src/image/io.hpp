// -*- C++ -*-
// io.hpp
//

#pragma once

#include "image.hpp"

#include <fmt/ostream.h>

#include <filesystem>
#include <fstream>
#include <string_view>

namespace fs = std::filesystem;

namespace lucid
{
template <typename T, std::size_t NC, typename Alloc>
void
write_ppm(const ScanlineImage<T, NC, Alloc>& img, const fs::path& filename)
{
    std::ofstream os(filename, std::ios_base::out | std::ios_base::binary);
    const auto [w, h] = img.res();
    fmt::print(os, "P6\n{} {}\n255\n", w, h);

    for(const auto pix: img)
    {
        const Vector<std::uint8_t, 3, std::array> rgb{downsample<std::uint8_t>(pix)};
        for(const std::uint8_t val: rgb) os << val;
    }
}

} // namespace lucid
