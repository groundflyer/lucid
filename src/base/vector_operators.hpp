// -*- C++ -*-
// vector.hpp --
//

#pragma once

// we don't use StaticSpan here, but if we don't include it
// compiler would be unable to find std::get specialization
// for Vector's Container
#include <utils/static_span.hpp>
#include <utils/math.hpp>

#include <array>
#include <limits>
#include <utility>
#include <numeric>
#include <algorithm>
#include <functional>
#include <type_traits>


namespace lucid
{
    template <typename T1, typename T2, std::size_t N,
			  template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2,
			  typename BinaryOperation>
    constexpr auto
    transform(BinaryOperation binary_op,
              const VectorType1<T1, N, Container1>& a,
			  const VectorType2<T2, N, Container2>& b) noexcept
    {
        using ElemType = typename std::decay_t<std::result_of_t<BinaryOperation(T1, T2)>>;
        using RetType = VectorType1<ElemType, N, std::array>;

        RetType ret{};

		for (std::size_t i = 0; i < N; ++i)
			ret[i] = binary_op(a[i], b[i]);

		return ret;
    }


    template <typename T, std::size_t N,
			  template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType,
			  typename UnaryOperation>
    constexpr auto
    transform(UnaryOperation unary_op,
              const VectorType<T, N, Container> & a) noexcept
    {
        using ElemType = typename std::decay_t<std::result_of_t<UnaryOperation(T)>>;
        using RetType = VectorType<ElemType, N, std::array>;

	    RetType ret{};

		for (std::size_t i = 0; i < N; ++i)
			ret[i] = unary_op(a[i]);

		return ret;
    }


    template <typename T, std::size_t N,
			  template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType,
			  typename BinaryOperation,
			  typename Init>
    constexpr auto
    reduce(BinaryOperation binary_op,
           const VectorType<T, N, Container>& a,
    	   Init init) noexcept
    {
    	for (std::size_t i = 0; i < N; ++i)
    	    init = binary_op(init, a[i]);

    	return init;
    }


    template <typename T1, typename T2, std::size_t N,
			  template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2,
			  typename BinaryOperation1,
			  typename BinaryOperation2,
			  typename Init>
    constexpr auto
    transform_reduce(BinaryOperation1 binary_op1,
					 BinaryOperation2 binary_op2,
                     const VectorType1<T1, N, Container1> & a,
					 const VectorType2<T2, N, Container2> & b,
					 Init init) noexcept
    {
    	for (std::size_t i = 0; i < N; ++i)
    	    init = binary_op2(init, binary_op1(a[i], b[i]));

    	return init;
    }


    template <typename T, std::size_t N,
			  template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType,
			  typename UnaryOperation,
			  typename BinaryOperation,
			  typename Init>
    constexpr auto
    transform_reduce(UnaryOperation unary_op,
					 BinaryOperation binary_op,
                     const VectorType<T, N, Container> & a,
					 Init init) noexcept
    {
		for (std::size_t i = 0; i < N; ++i)
			init = binary_op(init, unary_op(a[i]));

		return init;
    }



    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    length2(const VectorType<T, N, Container> & a) noexcept
    { return transform_reduce(pow<2, T>, std::plus<T>(), a, T{0}); }

    template <typename T, std::size_t N,
			  template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    length(const VectorType<T, N, Container> & a) noexcept
    { return math::sqrt(length2(a)); }


    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    normalize(const VectorType<T, N, Container> & a) noexcept
    {
    	const auto l = length(a);
        if constexpr(std::is_floating_point_v<T>)
            return !(almost_equal(l, T{1}, 5) || almost_equal(l, T{0}, 5)) ? a / l : a;
        else
            return !(l == 1 && l == 0) ? a / l : a;
    }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
    constexpr auto
    distance(const VectorType1<T, N, Container1> & a,
			 const VectorType2<T, N, Container2> & b)
    { return length(a - b); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    sum(const VectorType<T, N, Container> & a) noexcept
    { return reduce(std::plus<T>(), a, T{0}); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    product(const VectorType<T, N, Container> & a) noexcept
    { return reduce(std::multiplies<T>(), a, T{1}); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    all(const VectorType<T, N, Container> & a) noexcept
    { return reduce(std::logical_and<bool>(), a, true); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    any(const VectorType<T, N, Container> & a) noexcept
    { return reduce(std::logical_or<bool>(), a, false); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    avg(const VectorType<T, N, Container> & a) noexcept
    { return sum(a) / N; }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    max(const VectorType<T, N, Container> & a) noexcept
    { return reduce(static_cast<const T&(*)(const T&, const T&)>(std::max), a, std::numeric_limits<T>::min()); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    min(const VectorType<T, N, Container> & a) noexcept
    { return reduce(static_cast<const T&(*)(const T&, const T&)>(std::min), a, std::numeric_limits<T>::max()); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
    constexpr auto
    max(const VectorType1<T, N, Container1>& a,
        const VectorType2<T, N, Container2>& b) noexcept
    { return transform(static_cast<const T&(*)(const T&, const T&)>(std::max), a, b); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
    constexpr auto
    min(const VectorType1<T, N, Container1>& a,
        const VectorType2<T, N, Container2>& b) noexcept
    { return transform(static_cast<const T&(*)(const T&, const T&)>(std::min), a, b); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2,
              typename ULP>
    constexpr auto
    almost_equal(const VectorType1<T, N, Container1>& va,
                 const VectorType2<T, N, Container2>& vb,
                 const ULP ulp)
    { return transform([ulp](const T a, const T b){ return almost_equal(a, b, ulp); }, va, vb); }

    template <typename T, std::size_t N, typename ULP,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    almost_equal(const VectorType<T, N, Container>& va,
                 const T b,
                 const ULP ulp)
    { return transform([ulp, b](const T a){ return almost_equal(a, b, ulp); }, va); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    fit(const VectorType<T, N, Container>& v,
        const T minval,
        const T maxval) noexcept
    { return transform([minval, maxval](const T& val){ return fit(minval, maxval, val); }, v); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    abs(const VectorType<T, N, Container>& v) noexcept
    { return transform(static_cast<T(*)(T)>(math::abs), v); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    clamp(const VectorType<T, N, Container>& v,
          const T minval,
          const T maxval) noexcept
    { return transform([&](const T val){ return std::clamp(val, minval, maxval); }, v); }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    constexpr auto
    roll(const VectorType<T, N, Container>& v,
         const std::size_t shift) noexcept
    {
        VectorType<T, N, std::array> ret{};

        for (std::size_t i = 0; i < N; ++i)
            ret[i] = v[(i - shift) % N];

        return ret;
    }

    template <typename T, std::size_t N,
    	      template <typename, std::size_t> class Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    auto
    isfinite(const VectorType<T, N, Container>& v) noexcept
    { return transform(static_cast<bool(*)(T)>(std::isfinite), v); }

    // dot product
    template <typename T, std::size_t N,
			  template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
    constexpr auto
    dot(const VectorType1<T, N, Container1> & a,
    	const VectorType2<T, N, Container2> & b) noexcept
    { return transform_reduce(std::multiplies<T>(), std::plus<T>(), a, b, T{0}); }


    // N-dimensional cross product
    template <typename T, std::size_t N,
			  template <typename, std::size_t> class Container1,
			  template <typename, std::size_t> class Container2,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType1,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
    constexpr auto
    cross(const VectorType1<T, N, Container1> & a,
    	  const VectorType2<T, N, Container2> & b) noexcept
    {
    	VectorType1<T, N, std::array> ret{};

    	for (std::size_t i = 0; i < N; ++i)
    	    for (std::size_t j = 0; j < N; ++j)
				for (std::size_t k = 0; k < N; ++k)
					ret[i] += sgn(std::array<std::size_t, 3>({{i,j,k}})) * a[j] * b[k];

    	return ret;
    }

    template <typename T, std::size_t N,
              template <typename, std::size_t> typename Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    class ImmutableVectorOperators
    {
		static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

        constexpr const VectorType<T, N, Container>&
        this_vec() const noexcept
        { return static_cast<const VectorType<T, N, Container>&>(*this); }

    public:
		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator+(const VectorType2<T, N, Container2>& rhs) const noexcept
		{ return transform(std::plus<T>(), this_vec(), rhs); }
		constexpr auto
		operator+(const T& rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a + rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator-(const VectorType2<T, N, Container2>& rhs) const noexcept
		{ return transform(std::minus<T>(), this_vec(), rhs); }
		constexpr auto
		operator-(const T& rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a - rhs; }, this_vec()); }
		constexpr auto
		operator-() const noexcept
		{ return transform(std::negate<T>(), this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator*(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::multiplies<T>(), this_vec(), rhs); }
		constexpr auto
		operator*(const T & rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a * rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator/(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::divides<T>(), this_vec(), rhs); }
		constexpr auto
		operator/(const T & rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a / rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator==(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::equal_to<T>(), this_vec(), rhs); }
		constexpr auto
		operator==(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem == rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator!=(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::not_equal_to<T>(), this_vec(), rhs); }
		constexpr auto
		operator!=(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem != rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator>(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::greater<T>(), this_vec(), rhs); }
		constexpr auto
		operator>(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem > rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator<(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::less<T>(), this_vec(), rhs); }
		constexpr auto
		operator<(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem < rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator>=(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::greater_equal<T>(), this_vec(), rhs); }
		constexpr auto
		operator>=(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem >= rhs; }, this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr auto
		operator<=(const VectorType2<T, N, Container2> & rhs) const noexcept
		{ return transform(std::less_equal<T>(), this_vec(), rhs); }
		constexpr auto
		operator<=(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem <= rhs; }, this_vec()); }

        constexpr auto
        operator!() const noexcept
        { return transform(std::logical_not<T>(), this_vec()); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
        constexpr auto
        dot(const VectorType2<T, N, Container2>& rhs) const noexcept
        { return lucid::dot(this_vec(), rhs); }

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
        constexpr auto
        cross(const VectorType2<T, N, Container2>& rhs) const noexcept
        { return lucid::cross(this_vec(), rhs); }
    };


    template <typename T, std::size_t N,
              template <typename, std::size_t> typename Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType>
    class MutableVectorOperators
    {
		static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

        constexpr VectorType<T, N, Container>&
        this_vec() noexcept
        { return static_cast<VectorType<T, N, Container>&>(*this); }

    public:
		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr decltype(auto)
		operator+=(const VectorType2<T, N, Container2> & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] += rhs[i];

			return this_vec();
		}
		constexpr decltype(auto)
		operator+=(const T & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] += rhs;

			return this_vec();
		}

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr decltype(auto)
		operator-=(const VectorType2<T, N, Container2> & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] -= rhs[i];

			return this_vec();
		}
		constexpr decltype(auto)
		operator-=(const T & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] -= rhs;

			return this_vec();
		}

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr decltype(auto)
		operator*=(const VectorType2<T, N, Container2> & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] *= rhs[i];

			return this_vec();
		}
		constexpr decltype(auto)
		operator*=(const T & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] *= rhs;

			return this_vec();
		}

		template <template <typename, std::size_t> typename Container2,
                  template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType2>
		constexpr decltype(auto)
		operator/=(const VectorType2<T, N, Container2> & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] /= rhs[i];

			return this_vec();
		}
		constexpr decltype(auto)
		operator/=(const T & rhs) noexcept
		{
			for (std::size_t i = 0; i < N; ++i)
				this_vec()[i] /= rhs;

			return this_vec();
		}
    };


    template <std::size_t idx,
              typename OutType, std::size_t OutSize,
              typename InType,
              typename ... Tail>
    constexpr std::array<OutType, OutSize>&
    vector_constructor(std::array<OutType, OutSize>& out,
                       const InType& head,
                       Tail&& ... tail) noexcept
    {
        static_assert(idx < OutSize, "Too many elements.");

        out[idx] = static_cast<OutType>(head);

        if constexpr (sizeof...(tail) > 0)
            return vector_constructor<idx + 1>(out, std::forward<Tail>(tail)...);

        if constexpr (idx < (OutSize - 1))
            return vector_constructor<idx + 1>(out, head);

        if constexpr (sizeof...(tail) == 0 && (idx + 1) < OutSize)
            return vector_constructor<idx + 1>(out, OutType{0});

        return out;
    }

    template <std::size_t idx,
              typename OutType, std::size_t OutSize,
              typename InType, std::size_t InSize,
              template <typename, std::size_t> typename Container,
              template <typename, std::size_t, template <typename, std::size_t> typename> typename VectorType,
              typename ... Tail>
    constexpr std::array<OutType, OutSize>&
    vector_constructor(std::array<OutType, OutSize>& out,
                       const VectorType<InType, InSize, Container>& head,
                       Tail&& ... tail) noexcept
    {
        static_assert(idx < OutSize, "Too many elements.");

        const constexpr auto N = std::min(OutSize - idx, InSize);
        for(std::size_t i = 0; i < N; ++i)
            out[idx + i] = static_cast<OutType>(head[i]);

        if constexpr (sizeof...(tail) > 0)
            return vector_constructor<idx + N>(out, std::forward<Tail>(tail)...);

        // fill remaining with zeros
        if constexpr (sizeof...(tail) == 0 && (idx + N) < OutSize)
            return vector_constructor<idx + N>(out, OutType{0});

        return out;
    }
}
