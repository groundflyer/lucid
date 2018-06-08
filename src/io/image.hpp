// -*- C++ -*-
// film.hpp --
//

#pragma once

#include <core/types.hpp>

#include <string>
#include <memory>
#include <iterator>
#include <stdexcept>

namespace yapt
{
    template <size_t NChannels,
              typename Allocator = std::allocator<float>>
    class Image
    {
        static_assert(NChannels > 1);

        Res m_res;
        Allocator m_alloc = Allocator();
        float* p_data = nullptr;

        template <typename Iterator>
        class base_iterator
        {
        protected:
            float* p_data;
            const size_t m_size;
            size_t m_pos = 0;

        public:
            base_iterator() = delete;

            explicit
            base_iterator(float* data, size_t size, size_t pos = 0) :
                p_data(data), m_size(size), m_pos(pos) {}

            Iterator&
            operator++() noexcept
            {
                m_pos++;
                ASSERT(m_pos <= m_size, "Iterator out of range.");
                return static_cast<Iterator&>(*this);
            }

            Iterator&
            operator--() noexcept
            {
                m_pos--;
                ASSERT(m_pos >= 0, "Iterator out of range.");
                return static_cast<Iterator&>(*this);
            }

            Iterator
            operator+(const size_t rhs) const noexcept
            {
                ASSERT(m_pos + rhs <= m_size, "Iterator out of range.");
                return Iterator(p_data, m_size, m_pos + rhs);
            }

            Iterator
            operator-(const size_t rhs) const noexcept
            {
                ASSERT(m_pos - rhs >= 0, "Iterator out of range.");
                return Iterator(p_data, m_size, m_pos - rhs);
            }

            Iterator&
            operator+=(const size_t rhs) noexcept
            {
                ASSERT(m_pos + rhs <= m_size, "Iterator out of range.");
                m_pos += rhs;
                return static_cast<Iterator&>(*this);
            }

            Iterator&
            operator-=(const size_t rhs) noexcept
            {
                ASSERT(m_pos - rhs >= 0, "Iterator out of range.");
                m_pos -= rhs;
                return static_cast<Iterator&>(*this);
            }

            size_t
            operator-(const Iterator& rhs) const noexcept
            {
                ASSERT(p_data == rhs.p_data, "Iterator is not reachable.");
                return m_pos - rhs.m_pos;
            }

            bool
            operator==(const base_iterator& rhs) const noexcept
            { return p_data == rhs.p_data && m_size == rhs.m_size && m_pos == rhs.m_pos; }

            bool
            operator!=(const base_iterator& rhs) const noexcept
            { return p_data != rhs.p_data || m_size != rhs.m_size || m_pos != rhs.m_pos; }
        };

    public:
        class iterator : public base_iterator<iterator>
        {
            using Super = base_iterator<iterator>;

        public:
            using Super::Super;

            auto
            operator*() const noexcept
            { return Vector(ArrayView<float, NChannels>(Super::p_data + Super::m_pos * NChannels)); }
        };

        class const_iterator : public base_iterator<const_iterator>
        {
            using Super = base_iterator<const_iterator>;

        public:
            using Super::Super;

            const auto
            operator*() const noexcept
            { return Vector(ArrayView<float, NChannels>(const_cast<float*>(Super::p_data + Super::m_pos * NChannels))); }
        };

        explicit
        Image(const Allocator& alloc) : m_alloc(alloc) {}

        Image(const Res& res,
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
            rhs.m_res = Res(0);
        }

        Image&
        operator=(const Image& rhs)
        {
            if (size() != rhs.size())
            {
                for (size_t i = 0; i < size(); ++i)
                    std::allocator_traits<Allocator>::destroy(m_alloc, p_data + i);

                std::allocator_traits<Allocator>::deallocate(m_alloc, p_data, size());

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
        { return iterator(p_data, size() / NChannels); }

        auto
        end() noexcept
        { return iterator(p_data, size() / NChannels, size() / NChannels); }

        auto
        cbegin() const noexcept
        { return const_iterator(p_data, size() / NChannels); }

        auto
        cend() const noexcept
        { return const_iterator(p_data, size() / NChannels, size() / NChannels); }

        auto
        operator[](const size_t i) noexcept
        {
            ASSERT(i < size(), "Index out of range.");
            return Vector<float, NChannels, ArrayView>(ArrayView<float, NChannels>(p_data + i * NChannels));
        }

        const auto
        operator[](const size_t i) const noexcept
        {
            ASSERT(i < size(), "Index out of range.");
            return Vector(ArrayView<float, NChannels>(const_cast<float*>(p_data + i * NChannels)));
        }

        decltype(auto)
        at(const size_t i, const size_t j) noexcept
        {
            ASSERT(i <= m_res[0] && j <= m_res[1], "Index out of range.");
            return (*this)[i * m_res[1] + j];
        }

        decltype(auto)
        at(const size_t i, const size_t j) const noexcept
        {
            ASSERT(i <= m_res[0] && j <= m_res[1], "Index out of range.");
            return (*this)[i * m_res[1] + j];
        }

        auto
        size() const noexcept
        { return product(m_res) * NChannels; }

        const Res&
        res() const noexcept
        { return m_res; }
    };
}
