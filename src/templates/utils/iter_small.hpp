// -*- C++ -*-
// itertools.hpp --
//

#pragma once

#include <algorithm>


namespace yapt
{
    // functions for constant-sized array-like containers
    // with random-access iterators

    // binary transform
    template <typename A, typename B, size_t N,
	      template <typename, size_t> class Array,
	      typename F>
    constexpr decltype(auto)
    bmap(const F & func,
	 const Array<A, N> & a,
	 const Array<B, N> & b) noexcept
    {
	Array<decltype(func(a[0],b[0])), N> ret {};

	for (size_t i = 0; i < N; ++i)
	    ret[i] = func(a[i], b[i]);

	return ret;
    }

    // binary transform with one scalar arg
    template <typename A, typename B, size_t N,
	      template <typename, size_t> class Array,
	      typename F>
    constexpr decltype(auto)
    bmap(const F & func,
	 const Array<A, N> & a,
	 const B & b) noexcept
    {
	Array<decltype(func(a[0], b)), N> ret {};

	for (size_t i = 0; i < N; ++i)
	    ret[i] = func(a[i], b);

	return ret;
    }


    // unary transform
    template <typename T, size_t N,
	      template <typename, size_t> class Array,
	      typename F>
    constexpr decltype(auto)
    umap(const F & func,
	 const Array<T, N> & a) noexcept
    {
	Array<decltype(func(a[0])), N> ret {};

	for (size_t i = 0; i < N; ++i)
	    ret[i] = func(a[i]);

	return ret;
    }


    template <typename T, size_t N,
    	      template <typename, size_t> class Array,
	      typename F>
    constexpr decltype(auto)
    reduce(const F func,
	   const Array<T, N> & a) noexcept
    {
	static_assert(N >= 2, "Container size is too small!");

	auto ret = func(a[0], a[1]);

	for (size_t i = 2; i < N; ++i)
	    ret = func(ret, a[i]);

	return ret;
    }

    template <typename T, size_t N,
    	      template <typename, size_t> class Array>
    constexpr void
    fill_range(Array<T, N> & a, T start = 0, const T & step = 1) noexcept
    {
	for (size_t i = 0; i < N; i++)
	    {
		a[i] = start;
		start += step;
	    }
    }
}
