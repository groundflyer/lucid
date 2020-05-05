// -*- C++ -*-
// reconstruction.hpp
//

#pragma once

#include <image/image.hpp>
#include <image_reconstruction/film.hpp>

namespace lucid
{
using Sample = std::pair<Vec2, RGB>;

struct window_scanline_iterator
{
    const Vec2u       offset;
    scanline_iterator iter;

    window_scanline_iterator() = delete;
    window_scanline_iterator&
    operator=(const window_scanline_iterator&) = delete;

    constexpr window_scanline_iterator(const Vec2u& offset_, const Vec2u& res, const unsigned pos) :
        offset(offset_), iter(res, pos)
    {
    }

    constexpr Vec2u operator*() noexcept { return offset + *iter; }

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

template <typename Image>
constexpr auto
filter_iterate(const Film<Image>& film, const Vec2& pos, const real filter_radius) noexcept
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

    const real  half_ratio = film._ratio * 0.5_r;
    const Vec2u bmin = film.pixel_index(lucid::max(pos - filter_radius, Vec2(-half_ratio, -0.5_r)));
    const Vec2u bmax = film.pixel_index(lucid::min(pos + filter_radius, Vec2(half_ratio, 0.5_r)));
    return iter_proxy{bmin, (bmax - bmin) + 1u};
}

struct TriangleFilter
{
    real radius;
    real invert_radius;

    constexpr TriangleFilter(const real& _radius) : radius(_radius), invert_radius(1_r / _radius) {}

    constexpr real
    operator()(const Vec2& pixel_ssp, const Vec2& sample_pos) const noexcept
    {
        const real dist = distance(pixel_ssp, sample_pos);
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
    operator()(const RGBA_<Container>& old_rgba, const Vec2& pixel_ssp, const Sample& sample) const
        noexcept
    {
        const auto& [sample_pos, sample_val] = sample;
        const real weight                    = filter(pixel_ssp, sample_pos);
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
    operator()(const RGBA_<Container>&, const Vec2& pixel_ssp, const Sample& sample) const noexcept
    {
        const auto& [sample_pos, sample_val] = sample;
        return RGBA(sample_val * filter(pixel_ssp, sample_pos), 1_r);
    }
};

template <typename Image, typename Updater>
Film<Image>&
sample_based_region_update(Film<Image>& film, Updater&& update, const Sample& sample) noexcept
{
    for(const Vec2u pidx: filter_iterate(film, sample.first, update.filter.radius))
    {
        const Vec2     pixel_ssp = film.sample_space(pidx);
        decltype(auto) pixel_val = film.img[pidx];
        pixel_val                = update(pixel_val, pixel_ssp, sample);
    }
    return film;
}

template <typename Image, typename Updater>
Film<Image>&
sample_based_singular_update(Film<Image>& film, Updater&& update, const Sample& sample) noexcept
{
    const auto& [sample_pos, sample_val] = sample;
    Vec2u pidx                           = film.pixel_index(sample_pos);
    auto& [i, j]                         = pidx;
    const auto [w, h]                    = film.img.res();
    i                                    = std::clamp(i, 0u, w);
    j                                    = std::clamp(j, 0u, h);
    const Vec2     pixel_ssp             = film.sample_space(pidx);
    decltype(auto) pixel_val             = film.img[pidx];
    pixel_val                            = update(pixel_val, pixel_ssp, sample);
    return film;
}
} // namespace lucid
