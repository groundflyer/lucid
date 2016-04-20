// -*- C++ -*-

#include "film.h"
#include <algorithm>


namespace yapt
{
    Film::Film() {}

    Film::Film(const unsigned & num_channels,
	       const Vec2u & res) : _num_channels(num_channels), _res(res)
    {
	_size = res.x() * res.y() * num_channels;
	_data = new float [_size];
    }

    Film::Film(const Film & rhs)
    {
	_num_channels = rhs._num_channels;
	_res = rhs._res;
	_size = rhs._size;

	_data = new float [_size];

	std::copy(rhs.cbegin(), rhs.cend(), _data);
    }

    Film::Film(Film && rhs)
    {
	_num_channels = rhs._num_channels;
	_res = rhs._res;
	_size = rhs._size;

	_data = rhs._data;
	rhs._data = nullptr;
    }

    Film &
    Film::operator=(const Film & rhs) noexcept
    {
	if (this == &rhs)
	    return *this;

	_num_channels = rhs._num_channels;
	_res = rhs._res;

	if (_size != rhs._size)
	    {
		_size = rhs._size;
		delete [] _data;
		_data = new float [_size];
	    }

	std::copy(rhs.cbegin(), rhs.cend(), _data);

	return *this;
    }

    // virtual
    Film::~Film()
    { delete [] _data; }

    const float*
    Film::begin() const noexcept
    { return _data; }

    const float*
    Film::end() const noexcept
    { return (_data + _size); }

    const float*
    Film::cbegin() const noexcept
    { return _data; }

    const float*
    Film::cend() const noexcept
    { return (_data + _size); }

    float*
    Film::begin() noexcept
    { return _data; }

    float*
    Film::end() noexcept
    { return (_data + _size); }

    float&
    Film::operator[](const size_t & i) noexcept
    {
	ASSERT(i < _size);
	return _data[i];
    }

    const float&
    Film::operator[](const size_t & i) const noexcept
    {
	ASSERT(i < _size);
	return _data[i];
    }

    size_t
    Film::size() const noexcept
    { return _size; }

    unsigned
    Film::num_channels() const noexcept
    { return _num_channels; }

    Vec2u
    Film::res() const noexcept
    { return _res; }

    void
    Film::contribute(const Vec2u & pos,
		     float * channels) noexcept
    {
	ASSERT(pos.x() <= _res.x() || pos.y() <= _res.y());

	const size_t offset = _num_channels * (_res.x()*pos.y() + pos.x());

	ASSERT(offset <= _size);

	std::copy(channels, channels + _num_channels, _data + offset);
    }
}
