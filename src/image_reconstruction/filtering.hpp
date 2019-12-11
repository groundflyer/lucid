// -*- C++ -*-
// reconstruction.hpp
//

#include <image/image.hpp>

#pragma once

namespace lucid
{
using Sample = std::pair<Vec2, RGB>;

struct window_scanline_iterator
{
    const Vec2u       start;
    scanline_iterator iter;

    window_scanline_iterator() = delete;
    window_scanline_iterator&
    operator=(const window_scanline_iterator&) = delete;

    constexpr window_scanline_iterator(const Vec2u& start_, const Vec2u& res, const unsigned pos) :
        start(start_), iter(res, pos)
    {
    }

    constexpr Vec2u operator*() noexcept { return start + *iter; }

    constexpr window_scanline_iterator&
    operator++() noexcept
    {
        ++iter;
        return *this;
    }

    constexpr bool
    operator!=(const window_scanline_iterator& rhs) const noexcept
    {
        return iter != rhs.iter;
    }

    constexpr bool
    operator<(const window_scanline_iterator& rhs) const noexcept
    {
        return iter < rhs.iter;
    }
};

// convert image coordinates to device coordinates (-0.5 to 0.5)
constexpr Vec2
device_coords(const Vec2& pos, const Vec2& res, const Vec2& size) noexcept
{
    return (pos - res * 0.5_r) / res + size * 0.5_r;
}

constexpr Vec2
device_coords(const Vec2u& pos, const Vec2& res, const Vec2& size) noexcept
{
    return device_coords(Vec2(pos), res, size);
}

constexpr Vec2u
pixel_pos(const Vec2& ndc, const Vec2& res) noexcept
{
    return Vec2u(transform(static_cast<real (*)(real)>(math::round), (ndc + 0.5_r) * res));
}

constexpr std::pair<Vec2u, Vec2u>
filter_bound(const Vec2& pos, const Vec2& res, const Vec2& pixel_size, const real radius) noexcept
{
    const Vec2 bmin = lucid::max(pos - radius, Vec2(-0.5_r));
    const Vec2 bmax = lucid::min(pos + radius, Vec2(0.5_r) - pixel_size);
    return std::pair{pixel_pos(bmin, res), pixel_pos(bmax, res)};
}

constexpr auto
filter_iterate(const Vec2& pos, const Vec2& res, const Vec2& pixel_size, const real radius) noexcept
{
    struct iter_proxy
    {
        const Vec2u offset;
        const Vec2u res;

        constexpr window_scanline_iterator
        begin() const noexcept
        {
            return window_scanline_iterator(offset, res, 0u);
        }

        constexpr window_scanline_iterator
        end() const noexcept
        {
            return window_scanline_iterator(offset, res, product(res));
        }
    };

    const auto [bmin, bmax] = filter_bound(pos, res, pixel_size, radius);
    return iter_proxy{bmin, (bmax - bmin) + 1u};
}

// radius of pixel bounding sphere
real
pixel_radius(const Vec2 pixel_size) noexcept
{
    return math::sqrt(sum(pow<2>(pixel_size))) * 0.5_r;
}

struct TriangleFilter
{
    real radius;
    real invert_radius;

    constexpr TriangleFilter(const real& _radius) : radius(_radius), invert_radius(1_r / _radius) {}

    constexpr real
    operator()(const Vec2& pixel_pos, const Vec2& sample_pos) const noexcept
    {
        const real dist = distance(pixel_pos, sample_pos);
        const real norm = dist * invert_radius;
        return 1_r - std::min(1_r, norm);
    }
};

template <typename Filter>
struct PixelUpdate
{
    const Filter filter;

    constexpr PixelUpdate(Filter _filter) : filter(_filter) {}

    template <template <typename, std::size_t> typename Container>
    constexpr RGBA
    operator()(const RGBA_<Container>& old_rgba, const Vec2& pixel_pos, const Sample& sample) const
        noexcept
    {
        const auto& [sample_pos, sample_val] = sample;
        const real weight                    = filter(pixel_pos, sample_pos);
        RGB        old_color(old_rgba);
        const real old_weight = old_rgba.template get<3>();
        const real new_weight = old_weight + weight;
        if(new_weight > 0_r)
            old_color = (old_color * old_weight + sample_val * weight) / new_weight;
        return RGBA(old_color, new_weight);
    }
};

template <typename Filter>
struct PixelReset
{
    const Filter filter;

    constexpr PixelReset(Filter _filter) : filter(_filter) {}

    template <template <typename, std::size_t> typename Container>
    constexpr RGBA
    operator()(const RGBA_<Container>&, const Vec2& pixel_pos, const Sample& sample) const noexcept
    {
        const auto& [sample_pos, sample_val] = sample;
        return RGBA(sample_val * filter(pixel_pos, sample_pos), 1_r);
    }
};

template <typename Updater>
void update_pixels(ScanlineImage<real, 4>& img,
                   Updater&&               update,
                   const Vec2&             res,
                   const Vec2&             pixel_size,
                   const Sample&           sample) noexcept
{
    const auto& [sample_pos, sample_val] = sample;
    // for(auto it = img.begin(); it != img.end(); ++it)
    // {
    //     const Vec2 pp = device_coords(it.pos(), res, pixel_size);
    //     *it           = update(*it, pp, sample);
    // }
    // for(auto it = img.begin(); it != img.end(); ++it) *it = RGBA(1_r, 0_r, 0_r, 1_r);

    // const auto [bmin, bmax] = filter_bound(sample_pos, res, pixel_size, update.filter.radius);
    // const auto end          = scanline_iterator(scanline_iterator::end{}, bmin, bmax);
    for(const Vec2u pos: filter_iterate(sample_pos, res, pixel_size, update.filter.radius))
    {
        decltype(auto) pixel_val = img[pos];
        // pixel_val                = RGBA(sample_val);
        const Vec2 pp = device_coords(pos, res, pixel_size);
        pixel_val     = update(pixel_val, pp, sample);
    }
}
} // namespace lucid
