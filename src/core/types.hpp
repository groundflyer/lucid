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



    // Temporal container for temporal variables
    template <typename T, size_t N>
    using TempContainer = std::array<T, N>;


    template <template <typename, size_t> class Container>
    using Vec2 = Vector<real, 2, Container>;

    template <template <typename, size_t> class Container>
    using Vec3 = Vector<real, 3, Container>;

    template <template <typename, size_t> class Container>
    using Vec4 = Vector<real, 4, Container>;

    template <template <typename, size_t> class Container>
    using RGB = Vector<float, 3, Container>;

    template <template <typename, size_t> class Container>
    using RGBA = Vector<float, 4, Container>;


    template <template <typename, size_t> class Container>
    using Mat2 = Matrix<real, 2, Container>;

    template <template <typename, size_t> class Container>
    using Mat3 = Matrix<real, 3, Container>;

    template <template <typename, size_t> class Container>
    using Mat4 = Matrix<real, 4, Container>;


    template <template <typename, size_t> class Container>
    class Point : public Vec3<Container>
    {
	using Super = Vec3<Container>;

    public:
	using Super::Super;
    };


    template <template <typename, size_t> class Container>
    class Normal : public Vec3<Container>
    {
	using Super = Vec3<Container>;

    public:
	template <class ... Types>
	explicit constexpr
	Normal(Types && ... args) : Super(std::forward<Types>(args)...)
	{ Super::normalize(); }

	template <class ... Types>
	constexpr Normal&
	operator=(Types && ... rhs) noexcept
	{
	    Super::operator=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal&
	operator+=(Types && ... rhs) noexcept
	{
	    Super::operator+=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal&
	operator-=(Types && ... rhs) noexcept
	{
	    Super::operator-=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal&
	operator*=(Types && ... rhs) noexcept
	{
	    Super::operator*=(std::forward<Types>(rhs)...);
	    Super::normalize();
	    return *this;
	}

	template <class ... Types>
	constexpr Normal&
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
}
