// -*- C++ -*-
// image.cpp
#include <image/image.hpp>
#include <image/io.hpp>
#include <utils/logging.hpp>

using namespace lucid;

template <typename Img>
int
iter_test(Img&& img) noexcept
{
    int errors = 0;
    for(auto it = img.begin(); it != img.end(); ++it)
    {
        const Vec2u pos        = it.pos();
        const auto& [i, j]     = pos;
        decltype(auto) cur_val = *it;
        decltype(auto) pos_val = img[pos];
        decltype(auto) ij_val  = img.at(i, j);
        errors += &cur_val[0] != &pos_val[0];
        errors += &cur_val[0] != &ij_val[0];
    }
    return errors;
}

template <typename Img>
void
fill(Img& img) noexcept
{
    const auto [w, h] = img.res();
    const float fw    = static_cast<float>(w);
    const float fh    = static_cast<float>(h);
    for(std::size_t y = 0u; y < h; ++y)
        for(std::size_t x = 0u; x < w; ++x)
        {
            decltype(auto) ref  = img[Vec2u(x, y)];
            const float    xval = static_cast<float>(x) / fw;
            const float    yval = static_cast<float>(y) / fh;
            ref[0]              = xval;
            ref[1]              = yval;
            ref[2]              = 1;
        }
}

template <typename Img>
int
iter_write_test(Img& img) noexcept
{
    fill(img);

    int errors = 0;
    for(auto it = img.begin(); it != img.end(); ++it)
    {
        const Vec2u    pos      = it.pos();
        decltype(auto) ref      = *it;
        const auto [xval, yval] = Vec2(pos) / Vec2(img.res());
        errors += xval != ref[0];
        errors += yval != ref[1];
    }

    return errors;
}

template <typename Img>
bool
ppm_write_test(const Img& img) noexcept
{
    fs::path filename("img.ppm");
    write_ppm(img, filename);
    return !(fs::exists(filename) || (fs::file_size(filename) > 0));
}

int
main()
{
    Logger logger(Logger::DEBUG);

    ScanlineImage<float, 4>       img(Vec2u(20u, 10u));
    const ScanlineImage<float, 4> cimg(Vec2u(10u));
    int                           errors = iter_test(img) + iter_test(cimg);
    errors += iter_write_test(img);
    errors += ppm_write_test(img);

    if(errors)
        logger.error("Image test fails with {} errors", errors);
    else
        logger.info("Image test successfull");

    return errors;
}
