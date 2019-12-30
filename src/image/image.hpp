// -*- C++ -*-
// film.hpp --
//

#pragma once

#include <base/types.hpp>

#include <iterator>
#include <memory>
#include <random>

namespace lucid
{
struct scanline_iterator
{
    const unsigned size;
    const unsigned width;
    unsigned       pos;

    scanline_iterator() = delete;
    scanline_iterator&
    operator=(const scanline_iterator&) = delete;

    constexpr scanline_iterator(const Vec2u& res, unsigned _pos) :
        size(product(res)), width(res.get<0>()), pos(_pos)
    {
    }

    constexpr Vec2u operator*() const noexcept { return Vec2u{pos % width, pos / width}; }

    constexpr scanline_iterator&
    operator++() noexcept
    {
        ++pos;
        return *this;
    }

    constexpr scanline_iterator
    operator++(int) noexcept
    {
        scanline_iterator tmp(*this);
                          operator++();
        return tmp;
        ;
    }

    constexpr bool
    operator!=(const scanline_iterator& rhs) const noexcept
    {
        return pos != rhs.pos;
    }

    constexpr bool
    operator<(const scanline_iterator& rhs) const noexcept
    {
        return pos < rhs.pos;
    }
};

// num_pixels for number of pixels vectors<T, NC>
// size for number of elements of type T
template <typename T, std::size_t NC, typename Allocator = std::allocator<T>>
class ScanlineImage
{
    Vec2u     m_res;
    Allocator m_alloc = Allocator();
    T*        p_data  = nullptr;

  public:
    std::size_t
    pos(const std::size_t x, const std::size_t y) const noexcept
    {
        CHECK_INDEX(x, width());
        CHECK_INDEX(y, height());
        return NC * (y * width() + x);
    }

    template <bool Const = false>
    class _iterator
    {
        using ImageRef = std::conditional_t<Const, const ScanlineImage&, ScanlineImage&>;
        ImageRef          img;
        scanline_iterator iter;

      public:
        _iterator()            = delete;
        _iterator(_iterator&&) = delete;
        _iterator&
        operator=(const _iterator&) = delete;

        _iterator(ImageRef _img, const unsigned pos) : img(_img), iter(_img.res(), pos) {}

        _iterator(const _iterator& rhs) : img(rhs.img), iter(rhs.iter) {}

        Vec2u
        pos() const noexcept
        {
            return *iter;
        }

        _iterator&
        operator++() noexcept
        {
            ++iter;
            return *this;
        }

        _iterator
        operator++(int) noexcept
        {
            _iterator tmp(*this);
                      operator++();
            return tmp;
        }

        bool
        operator!=(const _iterator& rhs) const noexcept
        {
            return iter != rhs.iter;
        }

        decltype(auto) operator*() const noexcept
        {
            CHECK_INDEX(iter.pos, img.size());
            return img[*iter];
        }
    };

    using iterator       = _iterator<false>;
    using const_iterator = _iterator<true>;

    explicit ScanlineImage(const Allocator& alloc) : m_alloc(alloc) {}

    ScanlineImage(const Vec2u& res, const Allocator& alloc = Allocator()) :
        m_res(res), m_alloc(alloc)
    {
        p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());

        for(std::size_t i = 0; i < size(); ++i)
            std::allocator_traits<Allocator>::construct(m_alloc, p_data + i);
    }

    ScanlineImage(const ScanlineImage& rhs) : m_res(rhs.m_res), m_alloc(rhs.m_alloc)
    {
        p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());
        std::copy(rhs.p_data, rhs.p_data + rhs.size(), p_data);
    }

    ScanlineImage(ScanlineImage&& rhs) :
        m_alloc(std::move(rhs.m_alloc)), m_res(rhs.m_res), p_data(rhs.p_data)
    {
        rhs.p_data = nullptr;
        rhs.m_res  = Vec2u(0u);
    }

    ScanlineImage&
    operator=(const ScanlineImage& rhs)
    {
        if(&rhs == this) return *this;

        if(size() != rhs.size())
        {
            if(p_data)
            {
                for(std::size_t i = 0; i < size(); ++i)
                    std::allocator_traits<Allocator>::destroy(m_alloc, p_data + i);

                std::allocator_traits<Allocator>::deallocate(m_alloc, p_data, size());
            }

            m_res  = rhs.m_res;
            p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());
        }

        std::copy(rhs.p_data, rhs.p_data + rhs.size(), p_data);

        return *this;
    }

    ~ScanlineImage() noexcept
    {
        for(std::size_t i = 0; i < size(); ++i)
            std::allocator_traits<Allocator>::destroy(m_alloc, p_data + i);

        std::allocator_traits<Allocator>::deallocate(m_alloc, p_data, size());
    }

    iterator
    begin() noexcept
    {
        return iterator(*this, 0u);
    }

    iterator
    end() noexcept
    {
        return iterator(*this, num_pixels());
    }

    const_iterator
    begin() const noexcept
    {
        return const_iterator(*this, 0u);
    }

    const_iterator
    end() const noexcept
    {
        return const_iterator(*this, num_pixels());
    }

    const_iterator
    cbegin() const noexcept
    {
        return const_iterator(*this, 0u);
    }

    const_iterator
    cend() const noexcept
    {
        return const_iterator(*this, num_pixels());
    }

    decltype(auto) operator[](const Vec2u& pos) noexcept
    {
        const auto& [x, y] = pos;
        return at(x, y);
    }

    decltype(auto) operator[](const Vec2u& pos) const noexcept
    {
        const auto& [x, y] = pos;
        return at(x, y);
    }

    decltype(auto)
    at(const std::size_t i) noexcept
    {
        CHECK_INDEX(i, size());
        return Vector(StaticSpan<T, NC>(p_data + i));
    }

    decltype(auto)
    at(const std::size_t i) const noexcept
    {
        CHECK_INDEX(i, size());
        return Vector(StaticSpan<T, NC>(p_data + i));
    }

    decltype(auto)
    at(const std::size_t x, const std::size_t y) noexcept
    {
        return at(pos(x, y));
    }

    decltype(auto)
    at(const std::size_t x, const std::size_t y) const noexcept
    {
        return at(pos(x, y));
    }

    std::size_t
    width() const noexcept
    {
        return m_res.get<0>();
    }

    std::size_t
    height() const noexcept
    {
        return m_res.get<1>();
    }

    std::size_t
    num_pixels() const noexcept
    {
        return product(m_res);
    }

    std::size_t
    size() const noexcept
    {
        return num_pixels() * NC;
    }

    const Vec2u&
    res() const noexcept
    {
        return m_res;
    }

    const T*
    data() const noexcept
    {
        return p_data;
    }
};
} // namespace lucid
