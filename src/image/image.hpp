// -*- C++ -*-
// film.hpp --
//

#pragma once

#include <base/types.hpp>

#include <memory>
#include <iterator>

namespace yapt
{
    // num_pixels for number of pixels vectors<T, NC>
    // size for number of elements of type T
    template <typename T, size_t NC,
              typename Allocator = std::allocator<T>>
    class Image
    {
        Vec2u m_res;
        Allocator m_alloc = Allocator();
        T* p_data = nullptr;

        size_t
        pos(const size_t row, const size_t column) const noexcept
        {
            CHECK_INDEX(row, height());
            CHECK_INDEX(column, width());
            return NC * (row * width() + column);
        }

        template <typename Iterator>
        class base_iterator
        {
        protected:
            T* p_data;
            Vec2u m_res;
            size_t m_pos = 0;

            auto
            num_pixels() const noexcept
            { return product(m_res); }

        public:
            base_iterator() = delete;

            explicit
            base_iterator(T* data, const Vec2u& res, size_t pos = 0) :
                p_data(data), m_res(res), m_pos(pos) {}

            auto
            pos() const noexcept
            {
                const auto x = m_pos % m_res[0];
                const auto y = (m_pos - x) / m_res[0];
                return Vec2u(x, y);
            }

            Iterator&
            operator++() noexcept
            {
                m_pos++;
                return static_cast<Iterator&>(*this);
            }

            Iterator&
            operator--() noexcept
            {
                m_pos--;
                return static_cast<Iterator&>(*this);
            }

            Iterator
            operator+(const size_t rhs) const noexcept
            { return Iterator(p_data, m_res, m_pos + rhs); }

            Iterator
            operator-(const size_t rhs) const noexcept
            { return Iterator(p_data, m_res, m_pos - rhs); }

            Iterator&
            operator+=(const size_t rhs) noexcept
            {
                m_pos += rhs;
                return static_cast<Iterator&>(*this);
            }

            Iterator&
            operator-=(const size_t rhs) noexcept
            {
                m_pos -= rhs;
                return static_cast<Iterator&>(*this);
            }

            size_t
            operator-(const Iterator& rhs) const noexcept
            { return m_pos - rhs.m_pos; }

            bool
            operator==(const base_iterator& rhs) const noexcept
            { return p_data == rhs.p_data && num_pixels() == rhs.num_pixels() && m_pos == rhs.m_pos; }

            bool
            operator!=(const base_iterator& rhs) const noexcept
            { return p_data != rhs.p_data || num_pixels() != rhs.num_pixels() || m_pos != rhs.m_pos; }
        };

    public:
        class iterator : public base_iterator<iterator>
        {
            using Super = base_iterator<iterator>;

        public:
            using Super::Super;

            auto
            operator*() const noexcept
            {
                CHECK_INDEX(Super::m_pos, Super::num_pixels());
                return Vector(ArrayView<T, NC>(Super::p_data + Super::m_pos * NC));
            }
        };

        class const_iterator : public base_iterator<const_iterator>
        {
            using Super = base_iterator<const_iterator>;

        public:
            using Super::Super;

            const auto
            operator*() const noexcept
            {
                CHECK_INDEX(Super::m_pos, Super::num_pixels());
                return Vector(ArrayView<T, NC>(const_cast<T*>(Super::p_data + Super::m_pos * NC)));
            }
        };

        class view
        {
            T* p_data;
            Vec2u m_res;

        public:
            view() = delete;

            view(T* data, const Vec2u& res) :
                p_data(data), m_res(Vec2u(res[0], 1)) {}

            iterator
            begin() const noexcept
            { return iterator(p_data, m_res); }

            iterator
            end() const noexcept
            { return iterator(p_data, m_res, m_res[0]); }

            const_iterator
            cbegin() const noexcept
            { return iterator(p_data, m_res); }

            const_iterator
            cend() const noexcept
            { return iterator(p_data, m_res, m_res[0]); }

            auto
            operator[](const size_t i) noexcept
            {
                CHECK_INDEX(i, m_num_pixels);
                return Vector(ArrayView<T, NC>(p_data + i * NC));
            }

            const auto
            operator[](const size_t i) const noexcept
            {
                CHECK_INDEX(i, m_num_pixels);
                return Vector(ArrayView<T, NC>(const_cast<T*>(p_data + i * NC)));
            }
        };

        explicit
        Image(const Allocator& alloc) : m_alloc(alloc) {}

        Image(const Vec2u& res,
              const Allocator& alloc = Allocator()) : m_res(res), m_alloc(alloc)
        {
            p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());

            for (size_t i = 0; i < size(); ++i)
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
            rhs.m_res = Vec2u(0);
        }

        Image&
        operator=(const Image& rhs)
        {
            if (size() != rhs.size())
            {
                if(p_data)
                {
                    for (size_t i = 0; i < size(); ++i)
                        std::allocator_traits<Allocator>::destroy(m_alloc, p_data + i);

                    std::allocator_traits<Allocator>::deallocate(m_alloc, p_data, size());
                }

                m_res = rhs.m_res;
                p_data = std::allocator_traits<Allocator>::allocate(m_alloc, size());
            }

            std::copy(rhs.p_data, rhs.p_data + rhs.size(), p_data);
        }

        virtual ~Image() noexcept
        {
            for (size_t i = 0; i < size(); ++i)
                std::allocator_traits<Allocator>::destroy(m_alloc, p_data + i);

            std::allocator_traits<Allocator>::deallocate(m_alloc, p_data, size());
        }

        auto
        begin() noexcept
        { return iterator(p_data, m_res); }

        auto
        end() noexcept
        { return iterator(p_data, m_res, num_pixels()); }

        auto
        begin() const noexcept
        { return const_iterator(p_data, m_res); }

        auto
        end() const noexcept
        { return const_iterator(p_data, m_res, num_pixels()); }

        auto
        cbegin() const noexcept
        { return const_iterator(p_data, m_res); }

        auto
        cend() const noexcept
        { return const_iterator(p_data, m_res, num_pixels()); }

        auto
        operator[](const size_t i) noexcept
        { return view(p_data + pos(i, 0), m_res); }

        const auto
        operator[](const size_t i) const noexcept
        { return view(p_data + pos(i, 0), m_res); }

        auto
        at(const size_t i) noexcept
        {
            CHECK_INDEX(i, num_pixels());
            return Vector(ArrayView<T, NC>(p_data + i * NC));
        }

        const auto
        at(const size_t i) const noexcept
        {
            CHECK_INDEX(i, num_pixels());
            return Vector(ArrayView<T, NC>(const_cast<T*>(p_data + i * NC)));
        }

        auto
        at(const size_t i, const size_t j) noexcept
        { return Vector(ArrayView<T, NC>(p_data + pos(i, j))); }

        const auto
        at(const size_t i, const size_t j) const noexcept
        { return Vector(ArrayView<T, NC>(const_cast<T*>(p_data + pos(i, j)))); }

        const size_t&
        width() const noexcept
        { return m_res[0]; }

        const size_t&
        height() const noexcept
        { return m_res[1]; }

        auto
        num_pixels() const noexcept
        { return product(m_res); }

        auto
        size() const noexcept
        { return num_pixels() * NC; }

        const Vec2u&
        res() const noexcept
        { return m_res; }

        const T*
        data() const noexcept
        { return p_data; }
    };
}
