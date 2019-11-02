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
// num_pixels for number of pixels vectors<T, NC>
// size for number of elements of type T
template <typename T, std::size_t NC, typename Allocator = std::allocator<T>>
class Image
{
    Vec2u     m_res;
    Allocator m_alloc = Allocator();
    T*        p_data  = nullptr;

    std::size_t
    pos(const std::size_t row, const std::size_t column) const noexcept
    {
        CHECK_INDEX(row, height());
        CHECK_INDEX(column, width());
        return NC * (row * width() + column);
    }

  public:
    class iterator
    {
        T*          p_data;
        Vec2u       m_res;
        std::size_t m_pos = 0;

        auto
        num_pixels() const noexcept
        {
            return product(m_res);
        }

      public:
        iterator(){};

        explicit iterator(T* data, const Vec2u& res, std::size_t pos = 0) :
            p_data(data), m_res(res), m_pos(pos)
        {
        }

        auto
        pos() const noexcept
        {
            const auto x = m_pos % m_res[0];
            const auto y = (m_pos - x) / m_res[0];
            return Vec2u(x, y);
        }

        iterator&
        operator++() noexcept
        {
            m_pos++;
            return *this;
        }

        iterator&
        operator--() noexcept
        {
            m_pos--;
            return *this;
        }

        iterator
        operator+(const std::size_t rhs) const noexcept
        {
            return iterator(p_data, m_res, m_pos + rhs);
        }

        iterator
        operator-(const std::size_t rhs) const noexcept
        {
            return iterator(p_data, m_res, m_pos - rhs);
        }

        iterator&
        operator+=(const std::size_t rhs) noexcept
        {
            m_pos += rhs;
            return *this;
        }

        iterator&
        operator-=(const std::size_t rhs) noexcept
        {
            m_pos -= rhs;
            return *this;
        }

        std::size_t
        operator-(const iterator& rhs) const noexcept
        {
            return m_pos - rhs.m_pos;
        }

        bool
        operator==(const iterator& rhs) const noexcept
        {
            return p_data == rhs.p_data && num_pixels() == rhs.num_pixels() && m_pos == rhs.m_pos;
        }

        bool
        operator!=(const iterator& rhs) const noexcept
        {
            return p_data != rhs.p_data || num_pixels() != rhs.num_pixels() || m_pos != rhs.m_pos;
        }

        decltype(auto) operator*() const noexcept
        {
            CHECK_INDEX(m_pos, num_pixels());
            return Vector(StaticSpan<T, NC>(p_data + m_pos * NC));
        }
    };

    class view
    {
        T*    p_data;
        Vec2u m_res;

      public:
        view() = delete;

        view(T* data, const Vec2u& res) : p_data(data), m_res(Vec2u(res[0], 1)) {}

        iterator
        begin() const noexcept
        {
            return iterator(p_data, m_res);
        }

        iterator
        end() const noexcept
        {
            return iterator(p_data, m_res, m_res[0]);
        }

        iterator
        cbegin() const noexcept
        {
            return iterator(p_data, m_res);
        }

        iterator
        cend() const noexcept
        {
            return iterator(p_data, m_res, m_res[0]);
        }

        auto operator[](const std::size_t i) noexcept
        {
            return Vector(StaticSpan<T, NC>(p_data + i * NC));
        }

        decltype(auto) operator[](const std::size_t i) const noexcept
        {
            return Vector(StaticSpan<T, NC>(p_data + i * NC));
        }
    };

    explicit Image(const Allocator& alloc) : m_alloc(alloc) {}

    Image(const Vec2u& res, const Allocator& alloc = Allocator()) : m_res(res), m_alloc(alloc)
    {
        p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());

        for(std::size_t i = 0; i < size(); ++i)
            std::allocator_traits<Allocator>::construct(m_alloc, p_data + i);
    }

    Image(const Image& rhs) : m_res(rhs.m_res), m_alloc(rhs.m_alloc)
    {
        p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());
        std::copy(rhs.p_data, rhs.p_data + rhs.size(), p_data);
    }

    Image(Image&& rhs) : m_alloc(std::move(rhs.m_alloc)), m_res(rhs.m_res), p_data(rhs.p_data)
    {
        rhs.p_data = nullptr;
        rhs.m_res  = Vec2u(0);
    }

    Image&
    operator=(const Image& rhs)
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

    ~Image() noexcept
    {
        for(std::size_t i = 0; i < size(); ++i)
            std::allocator_traits<Allocator>::destroy(m_alloc, p_data + i);

        std::allocator_traits<Allocator>::deallocate(m_alloc, p_data, size());
    }

    auto
    begin() noexcept
    {
        return iterator(p_data, m_res);
    }

    auto
    end() noexcept
    {
        return iterator(p_data, m_res, num_pixels());
    }

    auto
    begin() const noexcept
    {
        return iterator(p_data, m_res);
    }

    auto
    end() const noexcept
    {
        return iterator(p_data, m_res, num_pixels());
    }

    auto
    cbegin() const noexcept
    {
        return iterator(p_data, m_res);
    }

    auto
    cend() const noexcept
    {
        return iterator(p_data, m_res, num_pixels());
    }

    auto operator[](const std::size_t i) noexcept { return view(p_data + pos(i, 0), m_res); }

    decltype(auto) operator[](const std::size_t i) const noexcept
    {
        return view(p_data + pos(i, 0), m_res);
    }

    decltype(auto) operator[](const Vec2u pos) noexcept
    {
        const auto& [i, j] = pos;
        return at(i, j);
    }

    decltype(auto) operator[](const Vec2u pos) const noexcept
    {
        const auto& [i, j] = pos;
        return at(i, j);
    }

    auto
    at(const std::size_t i) noexcept
    {
        CHECK_INDEX(i, num_pixels());
        return Vector(StaticSpan<T, NC>(p_data + i * NC));
    }

    decltype(auto)
    at(const std::size_t i) const noexcept
    {
        CHECK_INDEX(i, num_pixels());
        return Vector(StaticSpan<T, NC>(p_data + i * NC));
    }

    auto
    at(const std::size_t i, const std::size_t j) noexcept
    {
        return Vector(StaticSpan<T, NC>(p_data + pos(i, j)));
    }

    decltype(auto)
    at(const std::size_t i, const std::size_t j) const noexcept
    {
        return Vector(StaticSpan<T, NC>(p_data + pos(i, j)));
    }

    std::size_t
    width() const noexcept
    {
        return m_res[0];
    }

    std::size_t
    height() const noexcept
    {
        return m_res[1];
    }

    auto
    num_pixels() const noexcept
    {
        return product(m_res);
    }

    auto
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
