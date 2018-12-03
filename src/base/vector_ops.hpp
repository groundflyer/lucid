// -*- C++ -*-
// vec_ops.hpp --
//

#pragma once

#include "math.hpp"

#include <array>
#include <limits>
#include <utility>
#include <numeric>
#include <algorithm>
#include <functional>
#include <type_traits>


namespace yapt
{
    // forward declaration
    template <typename T, size_t N,
              template <typename, size_t> class Container = std::array>
    class Vector;


    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2,
			  typename BinaryOperation>
    constexpr auto
    transform(const Vector<T, N, Container1> & a,
			  const Vector<T, N, Container2> & b,
			  BinaryOperation binary_op) noexcept
    {
		Vector<std::decay_t<std::result_of_t<BinaryOperation(T, T)>>, N> ret {};

		for (size_t i = 0; i < N; ++i)
			ret[i] = binary_op(a[i], b[i]);

		return ret;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container,
			  typename UnaryOperation>
    constexpr auto
    transform(const Vector<T, N, Container> & a,
			  UnaryOperation unary_op) noexcept
    {
		Vector<std::result_of_t<UnaryOperation(T)>, N> ret {};

		for (size_t i = 0; i < N; ++i)
			ret[i] = unary_op(a[i]);

		return ret;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container,
			  typename BinaryOperation = decltype(std::plus<T>()),
			  typename Init = std::decay_t<std::result_of_t<BinaryOperation(T, T)>>>
    constexpr auto
    reduce(const Vector<T, N, Container>& a,
    	   BinaryOperation binary_op = std::plus<T>(),
    	   Init init = static_cast<Init>(0)) noexcept
    {
    	for (size_t i = 0; i < N; ++i)
    	    init = binary_op(init, a[i]);

    	return init;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2,
			  typename BinaryOperation1 = decltype(std::multiplies<T>()),
			  typename BinaryOperation2 = decltype(std::plus<T>()),
			  typename Init = std::decay_t<std::result_of_t<BinaryOperation2(std::result_of_t<BinaryOperation1(T, T)>,std::result_of_t<BinaryOperation1(T, T)>)>>>
    constexpr auto
    transform_reduce(const Vector<T, N, Container1> & a,
					 const Vector<T, N, Container2> & b,
					 BinaryOperation1 binary_op1 = std::multiplies<T>(),
					 BinaryOperation2 binary_op2 = std::plus<T>(),
					 Init init = static_cast<Init>(0)) noexcept
    {
    	for (size_t i = 0; i < N; ++i)
    	    init = binary_op2(init, binary_op1(a[i], b[i]));

    	return init;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container,
			  typename UnaryOperation,
			  typename BinaryOperation = decltype(std::plus<T>()),
			  typename Init = std::decay_t<std::result_of_t<BinaryOperation(std::result_of_t<UnaryOperation(T)>,std::result_of_t<UnaryOperation(T)>)>>>
    constexpr auto
    transform_reduce(const Vector<T, N, Container> & a,
					 UnaryOperation unary_op,
					 BinaryOperation binary_op = std::plus<T>(),
					 Init init = static_cast<Init>(0)) noexcept
    {
		for (size_t i = 0; i < N; ++i)
			init = binary_op(init, unary_op(a[i]));

		return init;
    }


    // dot product
    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    dot(const Vector<T, N, Container1> & a,
    	const Vector<T, N, Container2> & b) noexcept
    { return transform_reduce(a, b); }


    // N-dimensional cross product
    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    cross(const Vector<T, N, Container1> & a,
    	  const Vector<T, N, Container2> & b) noexcept
    {
    	Vector<T, N> ret;

    	for (size_t i = 0; i < N; ++i)
    	    for (size_t j = 0; j < N; ++j)
				for (size_t k = 0; k < N; ++k)
					ret[i] += math::sgn(std::array<size_t, 3>({{i,j,k}})) * a[j] * b[k];

    	return ret;
    }

    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
              template <typename, size_t> class Container2>
    constexpr auto
    frontface(const Vector<T, N, Container1> & a,
              const Vector<T, N, Container2> & b) noexcept
    { return dot(a, b) < 0 ? -a : a; }

//     // 3-dimensional cross product
//     template <typename T,
// 	      template <typename, size_t> class Container1,
// 	      template <typename, size_t> class Container2>
//     constexpr Vector<T, 3, Container1>
//     cross(const Vector<T, 3, Container1> & a,
// 	  const Vector<T, 3, Container2> & b) noexcept
//     {
// 	Vector<T, 3, Container1> ret;

// #ifdef LEFT			// Lefthanded coordinate system
// 	ret.x() = a.z()*b.y() - a.y()*b.z();
// 	ret.y() = a.x()*b.z() - a.z()*b.x();
// 	ret.z() = a.y()*b.x() - a.x()*b.y();
// #else			// Righthanded
// 	ret.x() = a.y()*b.z() - a.z()*b.y();
// 	ret.y() = a.z()*b.x() - a.x()*b.z();
// 	ret.z() = a.x()*b.y() - a.y()*b.x();
// #endif	// LEFT

// 	return ret;
//     }


    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    length2(const Vector<T, N, Container> & a) noexcept
    { return transform_reduce(a, math::pow<2, T>); }

    template <typename T, size_t N,
			  template <typename, size_t> class Container>
    constexpr auto
    length(const Vector<T, N, Container> & a) noexcept
    { return math::sqrt(length2(a)); }


    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    normalize(const Vector<T, N, Container> & a) noexcept
    {
    	const auto l = length(a);
    	return l != 1 || l != 0 ? a / l : a;
    }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    distance(const Vector<T, N, Container1> & a,
			 const Vector<T, N, Container2> & b)
    { return length(a - b); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    sum(const Vector<T, N, Container> & a) noexcept
    { return reduce(a, std::plus<T>()); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    product(const Vector<T, N, Container> & a) noexcept
    { return reduce(a, std::multiplies<T>(), static_cast<T>(1)); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    all(const Vector<T, N, Container> & a) noexcept
    { return reduce(a, std::logical_and<bool>(), true); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    any(const Vector<T, N, Container> & a) noexcept
    { return reduce(a, std::logical_or<bool>()); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    avg(const Vector<T, N, Container> & a) noexcept
    { return sum(a) / N; }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    max(const Vector<T, N, Container> & a) noexcept
    { return reduce(a, static_cast<const T&(*)(const T&, const T&)>(std::max), std::numeric_limits<T>::min()); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    min(const Vector<T, N, Container> & a) noexcept
    { return reduce(a, static_cast<const T&(*)(const T&, const T&)>(std::min), std::numeric_limits<T>::max()); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    max(const Vector<T, N, Container1>& a,
        const Vector<T, N, Container2>& b) noexcept
    { return transform(a, b, static_cast<const T&(*)(const T&, const T&)>(std::max)); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    min(const Vector<T, N, Container1>& a,
        const Vector<T, N, Container2>& b) noexcept
    { return transform(a, b, static_cast<const T&(*)(const T&, const T&)>(std::min)); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    almost_equal(const Vector<T, N, Container1>& va,
                 const Vector<T, N, Container2>& vb,
                 const int ulp = N)
    { return transform(va, vb, [&](const T a, const T b)
                       { return math::almost_equal(a, b, ulp); }); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1>
    constexpr auto
    almost_equal(const Vector<T, N, Container1>& va,
                 const T vb,
                 const int ulp = N)
    { return transform(va, [&](const T a)
                       { return math::almost_equal(a, vb, ulp); }); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    fit(const Vector<T, N, Container>& v,
        const T& bottom = T(0),
        const T& top = T(1)) noexcept
    { return transform(v, [&](const T& val)
                       { return math::fit(val, bottom, top); }); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    abs(const Vector<T, N, Container>& v) noexcept
    { return transform(v, static_cast<T(*)(T)>(math::abs)); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    clamp(const Vector<T, N, Container>& v,
          const T& minval,
          const T& maxval) noexcept
    { return transform(v, [&](const int& val)
                       { return std::clamp(val, minval, maxval); }); }
}
