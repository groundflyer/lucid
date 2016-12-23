// -*- C++ -*-
// film.hpp --
//

#pragma once

#include <core/basic_types.hpp>


namespace yapt
{
    class Film
    {
	unsigned _num_channels = 4; // RGBA
	Vec2u _res;
	size_t _size = 0;
	float * _data = nullptr;

    public:
	Film();

	Film(const unsigned & num_channels,
	     const Vec2u & res);

	Film(const Film & rhs);

	Film(Film && rhs);

	Film &
	operator=(const Film & rhs) noexcept;

	virtual ~Film();

	const float*
	begin() const noexcept;

	const float*
	end() const noexcept;

	const float*
	cbegin() const noexcept;

	const float*
	cend() const noexcept;

	float*
	begin() noexcept;

	float*
	end() noexcept;

	float&
	operator[](const size_t & i) noexcept;

	const float&
	operator[](const size_t & i) const noexcept;

	size_t
	size() const noexcept;

	unsigned
	num_channels() const noexcept;

	Vec2u
	res() const noexcept;

	void
	contribute(const Vec2u & pos,
		   float * channels) noexcept;
    };
}
