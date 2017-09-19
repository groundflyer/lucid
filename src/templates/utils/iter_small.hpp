// -*- C++ -*-
// itertools.hpp --
//

#pragma once

#include <utility>
#include <functional>
#include <type_traits>


namespace yapt
{
    template <typename Vector>
    struct vector_value
    { typedef typename std::decay<decltype(std::declval<Vector>()[std::declval<size_t>()])>::type type; };

    template <typename Vector>
    using vector_value_t = typename vector_value<Vector>::type;

    template <typename Vector1, typename Vector2, typename BinaryOperation>
    struct result_of_binary_op
    { typedef typename std::result_of_t<BinaryOperation(vector_value_t<Vector1>, vector_value_t<Vector2>)> type; };

    template <typename T, size_t N,
	      template <typename, size_t> typename Container,
	      template <typename, size_t, template<typename,size_t> typename> typename Vector>
    constexpr size_t vector_size(const Vector<T, N, Container>&)
    { return N; }

    template <typename Vector1, typename Vector2, typename BinaryOperation>
    constexpr Vector1
    transform(const Vector1 & a,
	      const Vector2 & b,
	      BinaryOperation binary_op) noexcept
    {
	static_assert(vector_size(a) == vector_size(b), "Vector sizes don't match!");

	Vector1 ret {};

	for (size_t i = 0; i < a.size(); ++i)
	    ret[i] = binary_op(a[i], b[i]);

	return ret;
    }


    template <typename Vector, typename UnaryOperation>
    constexpr Vector
    transform(const Vector & a,
	      UnaryOperation unary_op) noexcept
    {
	Vector ret {};

	for (size_t i = 0; i < a.size(); ++i)
	    ret[i] = unary_op(a[i]);

	return ret;
    }


    template <typename Vector,
	      typename BinaryOperation = decltype(std::plus<vector_value_t<Vector>>()),
	      typename Init = result_of_binary_op_t<Vector, Vector, BinaryOperation>>
    constexpr auto
    reduce(const Vector & a,
    	   BinaryOperation binary_op = std::plus<vector_value_t<Vector>>(),
    	   Init init = static_cast<Init>(0)) noexcept
    {
    	for (size_t i = 0; i < a.size(); ++i)
    	    init = binary_op(init, a[i]);

    	return init;
    }
}
