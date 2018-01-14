// -*- C++ -*-
// types.hpp --
//

#pragma once

#include "vec.hpp"
#include "mat.hpp"


namespace yapt
{
#ifdef _DOUBLE_PRECISION
    typedef double real;
#else
    typedef float real;
#endif	// _DOUBLE_PRECISION

	real
	operator "" _r(long double d)
	{ return static_cast<real>(d); }

	real
	operator "" _r(unsigned long long d)
	{ return static_cast<real>(d); }

    // Temporal container for temporal variables
    template <typename T, size_t N>
    using StaticContainer = std::array<T, N>;


    template <template <typename, size_t> typename Container>
    using Vec2_ = Vector<real, 2, Container>;

    template <template <typename, size_t> typename Container>
    using Vec3_ = Vector<real, 3, Container>;

    template <template <typename, size_t> typename Container>
    using Vec4_ = Vector<real, 4, Container>;

    template <template <typename, size_t> typename Container>
    using RBG_ = Vector<float, 3, Container>;

    template <template <typename, size_t> typename Container>
    using RGBA_ = Vector<float, 4, Container>;


    template <template <typename, size_t> typename Container>
    using Mat2_ = Matrix<real, 2, 2, Container>;

    template <template <typename, size_t> typename Container>
    using Mat3_ = Matrix<real, 3, 3, Container>;

    template <template <typename, size_t> typename Container>
    using Mat4_ = Matrix<real, 4, 4, Container>;


    template <template <typename, size_t> typename Container>
    class Point_ : public Vec3_<Container>
    {
	using Super = Vec3_<Container>;

    public:
	using Super::Super;
    };


    template <template <typename, size_t> typename Container>
    class Normal_ : public Vec3_<Container>
    {
	using Super = Vec3_<Container>;

    public:
	template <class ... Types>
	explicit constexpr
	Normal_(Types && ... args) : Super(std::forward<Types>(args)...)
	{ Super::normalize(); }

	template <class ... Types>
	constexpr Normal_&
	operator=(Types && ... rhs) noexcept
	{
	    Super::operator=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal_&
	operator+=(Types && ... rhs) noexcept
	{
	    Super::operator+=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal_&
	operator-=(Types && ... rhs) noexcept
	{
	    Super::operator-=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal_&
	operator*=(Types && ... rhs) noexcept
	{
	    Super::operator*=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal_&
	operator/=(Types && ... rhs) noexcept
	{
	    Super::operator/=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	constexpr auto
	begin() noexcept = delete;

	constexpr auto
	end() noexcept = delete;

	constexpr real&
	operator[](const size_t i) noexcept = delete;

	constexpr real&
	at(const size_t i) noexcept = delete;
    };

    using Vec2 = Vec2_<StaticContainer>;
    using Vec3 = Vec3_<StaticContainer>;
    using Vec4 = Vec4_<StaticContainer>;
    using Point = Point_<StaticContainer>;
    using Normal = Normal_<StaticContainer>;
    using Mat2 = Mat2_<StaticContainer>;
    using Mat3 = Mat3_<StaticContainer>;
    using Mat4 = Mat4_<StaticContainer>;
}