// -*- C++ -*-
// types.hpp --
//

#pragma once

#include "vector.hpp"
#include "matrix.hpp"

#include <cstdint>


namespace yapt
{
#ifdef _DOUBLE_PRECISION
    typedef double real;
#else
    typedef float real;
#endif	// _DOUBLE_PRECISION

	constexpr real
	operator "" _r(long double d)
	{ return static_cast<real>(d); }

	constexpr real
	operator "" _r(unsigned long long d)
	{ return static_cast<real>(d); }


    template <template <typename, size_t> typename Container>
    using Vec2_ = Vector<real, 2, Container>;

    template <template <typename, size_t> typename Container>
    using Vec2u_ = Vector<unsigned, 2, Container>;

    template <template <typename, size_t> typename Container>
    using Vec3_ = Vector<real, 3, Container>;

    template <template <typename, size_t> typename Container>
    using Vec4_ = Vector<real, 4, Container>;

    template <template <typename, size_t> typename Container>
    using Vec3i_ = Vector<int, 3, Container>;

    template <template <typename, size_t> typename Container>
    using Mat2_ = Matrix<real, 2, 2, Container>;

    template <template <typename, size_t> typename Container>
    using Mat3_ = Matrix<real, 3, 3, Container>;

    template <template <typename, size_t> typename Container>
    using Mat4_ = Matrix<real, 4, 4, Container>;

    template <template <typename, size_t> typename Container>
    using RGB_ = Vector<float, 3, Container>;

    template <template <typename, size_t> typename Container>
    using RGBA_ = Vector<float, 4, Container>;

    template <template <typename, size_t> typename Container>
    using RGB8_ = Vector<std::uint8_t, 3, Container>;

    template <template <typename, size_t> typename Container>
    using RGBA8_ = Vector<std::uint8_t, 4, Container>;

    template <template <typename, size_t> typename Container>
    class Point_ : public Vec3_<Container>
    {
        using Super = Vec3_<Container>;

    public:
        using Super::Super;

		template <typename ... Ts>
		explicit constexpr
		Point_(Ts && ... args) : Super(std::forward<Ts>(args)...) {}

        template <typename T>
        constexpr Point_&
        operator=(T&& rhs) noexcept
        {
            Super::operator=(std::forward<T>(rhs));
            return *this;
        }

        template <typename T>
        constexpr auto
        operator+(T&& rhs) const noexcept
        { return Point_(Super::operator+(std::forward<T>(rhs))); }

        template <typename ... Ts>
        constexpr auto
        operator-(Ts && ... rhs) const noexcept
        { return Point_(Super::operator-(std::forward<Ts>(rhs)...)); }

        template <typename T>
        constexpr auto
        operator*(T&& rhs) const noexcept
        { return Point_(Super::operator*(std::forward<T>(rhs))); }

        template <typename T>
        constexpr auto
        operator/(T&& rhs) const noexcept
        { return Point_(Super::operator/(std::forward<T>(rhs))); }

        template <typename T>
        constexpr Point_&
        operator+=(T&& rhs) noexcept
        {
            Super::operator+=(std::forward<T>(rhs));
            return *this;
        }

        template <typename T>
        constexpr Point_&
        operator-=(T&& rhs) noexcept
        {
            Super::operator-=(std::forward<T>(rhs));
            return *this;
        }

        template <typename T>
        constexpr Point_&
        operator*=(T&& rhs) noexcept
        {
            Super::operator*=(std::forward<T>(rhs));
            return *this;
        }

        template <typename T>
        constexpr Point_&
        operator/=(T&& rhs) noexcept
        {
            Super::operator/=(std::forward<T>(rhs));
            return *this;
        }
    };

    template <template <typename, size_t> typename Container>
    Point_(Container<real, 3> &&) -> Point_<Container>;


    template <template <typename, size_t> typename Container>
    class Normal_ : public Vec3_<Container>
    {
		using Super = Vec3_<Container>;

		// inplace normalization
		constexpr void
		normalize() noexcept
		{
			auto l = length(*this);

			if (!(math::almost_equal(l, 1_r, 5) && math::almost_equal(l, 0_r, 5)))
				for (auto& elem : Super::m_data)
                    elem /= l;
		}

    public:
		template <typename ... Ts>
		explicit constexpr
		Normal_(Ts && ... args) : Super(std::forward<Ts>(args)...)
		{ normalize(); }

		template <typename T>
		constexpr Normal_&
		operator=(T&& rhs) noexcept
		{
			Super::operator=(std::forward<T>(rhs));
			normalize();
			return *this;
		}

        constexpr Normal_
        operator-() const noexcept
        { return Normal_(Super::operator-()); }

		template <typename T>
		constexpr auto
		operator-(T&& rhs) const noexcept
		{ return Super::operator-(std::forward<T>(rhs)); }

		template <typename T>
		constexpr Normal_&
		operator+=(T&& rhs) = delete;

		template <typename T>
		constexpr Normal_&
		operator-=(T&& rhs) = delete;

		template <typename T>
		constexpr Normal_&
		operator*=(T&& rhs) = delete;

		template <typename T>
		constexpr Normal_&
		operator/=(T&& rhs) = delete;

		constexpr auto
		begin() noexcept = delete;

		constexpr auto
		end() noexcept = delete;

		constexpr real&
		operator[](const size_t i) noexcept = delete;

		constexpr real&
		at(const size_t i) noexcept = delete;
    };

    template <template <typename, size_t> typename Container>
    Normal_(Container<real, 3> &&) -> Normal_<Container>;


    using Vec2 = Vec2_<std::array>;
    using Vec3 = Vec3_<std::array>;
    using Vec4 = Vec4_<std::array>;
    using Vec3i = Vec3i_<std::array>;
    using Vec2u = Vec2u_<std::array>;
    using Point = Point_<std::array>;
    using Normal = Normal_<std::array>;
    using Mat2 = Mat2_<std::array>;
    using Mat3 = Mat3_<std::array>;
    using Mat4 = Mat4_<std::array>;
    using RGB = RGB_<std::array>;
    using RGBA = RGBA_<std::array>;
    using RGB8 = RGB8_<std::array>;
    using RGBA8 = RGBA8_<std::array>;
}

namespace std
{
    template <template <typename, size_t> typename Container>
    class tuple_size<yapt::Point_<Container>>
    {
    public:
        static const constexpr size_t value = 3;
    };

    template<size_t I,
             template <typename, size_t> typename Container>
    class tuple_element<I, yapt::Point_<Container>>
    {
    public:
        using type = yapt::real;
    };

    template <template <typename, size_t> typename Container>
    class tuple_size<yapt::Normal_<Container>>
    {
    public:
        static const constexpr size_t value = 3;
    };

    template<size_t I,
             template <typename, size_t> typename Container>
    class tuple_element<I, yapt::Normal_<Container>>
    {
    public:
        using type = yapt::real;
    };
}
