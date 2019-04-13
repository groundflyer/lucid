// -*- C++ -*-
// vector.hpp --
//

#pragma once

// we don't use StaticSpan here, but if we don't include it
// compiler would be unable to find std::get specialization
// for Vector's Container
#include <utils/static_span.hpp>
#include <utils/debug.hpp>
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
    template <typename T, size_t N,
			  template <typename, size_t> typename Container = std::array>
    class Vector;

    // dot product
    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    dot(const Vector<T, N, Container1> & a,
    	const Vector<T, N, Container2> & b) noexcept
    { return transform_reduce(std::multiplies<T>(), std::plus<T>(), a, b, T{0}); }


    // N-dimensional cross product
    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    cross(const Vector<T, N, Container1> & a,
    	  const Vector<T, N, Container2> & b) noexcept
    {
    	Vector<std::decay_t<T>, N> ret;

    	for (size_t i = 0; i < N; ++i)
    	    for (size_t j = 0; j < N; ++j)
				for (size_t k = 0; k < N; ++k)
					ret[i] += math::sgn(std::array<size_t, 3>({{i,j,k}})) * a[j] * b[k];

    	return ret;
    }


    template <typename T, size_t N,
			  template <typename, size_t> typename Container>
    class Vector
    {
		static_assert(std::is_arithmetic<T>::value, "T is not an arithmetic type.");

    protected:
		using Data = Container<T, N>;

		Data m_data {};

        template <size_t idx>
        constexpr void
        unpack()
        {
            if constexpr (idx < N)
                for (auto i = idx; i < N; ++i)
                    m_data[i] = 0;
        }

        template <size_t idx, typename ... Ts>
        constexpr void
        unpack(const T& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            m_data[idx] = first;
            if constexpr (sizeof...(other) == 0 && idx < N-1)
                unpack<idx+1>(first);
            else
                unpack<idx+1>(std::forward<Ts>(other)...);
        }

		template <size_t idx, typename T1, size_t N1,
                  template <typename, size_t> typename Container1,
                  typename ... Ts>
        constexpr void
        unpack(const Vector<T1, N1, Container1>& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + i] = static_cast<T>(first[i]);
            unpack<idx + N1>(std::forward<Ts>(other)...);
        }

        template <size_t idx, typename T1, size_t N1,
                  typename ... Ts>
        constexpr void
        unpack(const Container<T1, N1>& first, Ts&& ... other)
        {
            static_assert(idx < N, "Too many elements.");
            for(size_t i = 0; i < std::min(N - idx, N1); ++i)
                m_data[idx + i] = static_cast<T>(first[i]);
            unpack<idx + N1>(std::forward<Ts>(other)...);
        }

    public:
		constexpr
		Vector() {}

		constexpr
		Vector(const Vector& rhs) : m_data(rhs.m_data) {}

		explicit constexpr
		Vector(Data && rhs) : m_data(std::forward<Data>(rhs)) {}

		explicit constexpr
		Vector(const Data& rhs) : m_data(rhs) {}

		explicit constexpr
		Vector(Data& rhs) : m_data(rhs) {}

		explicit constexpr
		Vector(T && rhs) { for (auto& i : m_data) i = rhs; }

        template <typename ... Ts>
        explicit constexpr
        Vector(Ts && ... rhs)
        { unpack<0>(std::forward<Ts>(rhs)...); }

		// // conversion constructor
		template <typename T2, size_t N2,
				  template <typename, size_t> typename Container2>
		constexpr
		Vector(const Vector<T2, N2, Container2> & rhs)
		{ for (size_t i = 0; i < std::min(N, N2); ++i) m_data[i] = static_cast<T>(rhs[i]); }

		constexpr Vector&
		operator=(const Vector & rhs) noexcept
		{
			m_data = rhs.m_data;
			return *this;
		}

		// between different containers we copy data manually
		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator=(const Vector<T, N, Container2> & rhs) noexcept
		{
			std::copy(rhs.cbegin(), rhs.cend(), begin());
			return *this;
		}

		constexpr Vector&
		operator=(const T & rhs) noexcept
		{
			for (T& i : m_data) i = rhs;
			return *this;
		}

		constexpr auto
		begin() noexcept
		{ return m_data.begin(); }
		constexpr auto
		end() noexcept
		{ return m_data.end(); }
		constexpr auto
		cbegin() const noexcept
		{ return m_data.cbegin(); }
		constexpr auto
		cend() const noexcept
		{ return m_data.cend(); }

		const constexpr T&
		operator[](const size_t i) const noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}
		constexpr T&
		operator[](const size_t i) noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}

		template <size_t I>
		constexpr T&
		get() noexcept
		{ return std::get<I>(m_data); }

		template <size_t I>
		constexpr const T&
		get() const noexcept
		{ return std::get<I>(m_data); }

		const constexpr T&
		at(const size_t i) const noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}
		constexpr T&
		at(const size_t i) noexcept
		{
            CHECK_INDEX(i, N);
			return m_data[i];
		}

		const constexpr Data&
		data() const noexcept
		{ return m_data; }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator+(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::plus<T>(), *this, rhs); }
		constexpr auto
		operator+(const T & rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a + rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator-(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::minus<T>(), *this, rhs); }
		constexpr auto
		operator-(const T & rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a - rhs; }, *this); }
		constexpr auto
		operator-() const noexcept
		{ return transform(std::negate<T>(), *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator*(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::multiplies<T>(), *this, rhs); }
		constexpr auto
		operator*(const T & rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a * rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator/(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::divides<T>(), *this, rhs); }
		constexpr auto
		operator/(const T & rhs) const noexcept
		{ return transform([&rhs](const T& a){ return a / rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator+=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] += rhs[i];

			return *this;
		}
		constexpr Vector&
		operator+=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] += rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator-=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] -= rhs[i];

			return *this;
		}
		constexpr Vector&
		operator-=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] -= rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator*=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] *= rhs[i];

			return *this;
		}
		constexpr Vector&
		operator*=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] *= rhs;

			return *this;
		}

		template <template <typename, size_t> typename Container2>
		constexpr Vector&
		operator/=(const Vector<T, N, Container2> & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] /= rhs[i];

			return *this;
		}

		constexpr Vector&
		operator/=(const T & rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i)
				m_data[i] /= rhs;

			return *this;
		}


		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator==(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::equal_to<T>(), *this, rhs); }
		constexpr auto
		operator==(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem == rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator!=(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::not_equal_to<T>(), *this, rhs); }
		constexpr auto
		operator!=(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem != rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator>(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::greater<T>(), *this, rhs); }
		constexpr auto
		operator>(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem > rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator<(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::less<T>(), *this, rhs); }
		constexpr auto
		operator<(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem < rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator>=(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::greater_equal<T>(), *this, rhs); }
		constexpr auto
		operator>=(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem >= rhs; }, *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		operator<=(const Vector<T, N, Container2> & rhs) const noexcept
		{ return transform(std::less_equal<T>(), *this, rhs); }
		constexpr auto
		operator<=(const T& rhs) const noexcept
		{ return transform([&rhs](const T& elem){ return elem <= rhs; }, *this); }

        constexpr auto
        operator!() const noexcept
        { return transform(std::logical_not<T>(), *this); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		dot(const Vector<T, N, Container2> & rhs) const noexcept
        { return yapt::dot(*this, rhs); }

		template <template <typename, size_t> typename Container2>
		constexpr auto
		cross(const Vector<T, N, Container2> & rhs) const noexcept
		{ return yapt::cross(*this, rhs); }

		constexpr auto
		size() const noexcept
		{ return N; }
    };

    template <typename T, size_t N,
			  template <typename, size_t> typename Container>
    Vector(Container<T, N> &&) -> Vector<T, N, Container>;

    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2,
			  typename BinaryOperation>
    constexpr auto
    transform(BinaryOperation binary_op,
              const Vector<T, N, Container1> & a,
			  const Vector<T, N, Container2> & b) noexcept
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
    transform(UnaryOperation unary_op,
              const Vector<T, N, Container> & a) noexcept
    {
		Vector<std::decay_t<std::result_of_t<UnaryOperation(T)>>, N> ret {};

		for (size_t i = 0; i < N; ++i)
			ret[i] = unary_op(a[i]);

		return ret;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container,
			  typename BinaryOperation,
			  typename Init>
    constexpr auto
    reduce(BinaryOperation binary_op,
           const Vector<T, N, Container>& a,
    	   Init init) noexcept
    {
    	for (size_t i = 0; i < N; ++i)
    	    init = binary_op(init, a[i]);

    	return init;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2,
			  typename BinaryOperation1,
			  typename BinaryOperation2,
			  typename Init>
    constexpr auto
    transform_reduce(BinaryOperation1 binary_op1,
					 BinaryOperation2 binary_op2,
                     const Vector<T, N, Container1> & a,
					 const Vector<T, N, Container2> & b,
					 Init init) noexcept
    {
    	for (size_t i = 0; i < N; ++i)
    	    init = binary_op2(init, binary_op1(a[i], b[i]));

    	return init;
    }


    template <typename T, size_t N,
			  template <typename, size_t> class Container,
			  typename UnaryOperation,
			  typename BinaryOperation,
			  typename Init>
    constexpr auto
    transform_reduce(UnaryOperation unary_op,
					 BinaryOperation binary_op,
                     const Vector<T, N, Container> & a,
					 Init init) noexcept
    {
		for (size_t i = 0; i < N; ++i)
			init = binary_op(init, unary_op(a[i]));

		return init;
    }




    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    length2(const Vector<T, N, Container> & a) noexcept
    { return transform_reduce(math::pow<2, T>, std::plus<T>(), a, T{0}); }

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
        if constexpr(std::is_floating_point_v<T>)
            return !(math::almost_equal(l, T{1}, 5) || math::almost_equal(l, T{0}, 5)) ? a / l : a;
        else
            return !(l == 1 && l == 0) ? a / l : a;
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
    { return reduce(std::plus<T>(), a, T{0}); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    product(const Vector<T, N, Container> & a) noexcept
    { return reduce(std::multiplies<T>(), a, T{1}); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    all(const Vector<T, N, Container> & a) noexcept
    { return reduce(std::logical_and<bool>(), a, true); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    any(const Vector<T, N, Container> & a) noexcept
    { return reduce(std::logical_or<bool>(), a, false); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    avg(const Vector<T, N, Container> & a) noexcept
    { return sum(a) / N; }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    max(const Vector<T, N, Container> & a) noexcept
    { return reduce(static_cast<const T&(*)(const T&, const T&)>(std::max), a, std::numeric_limits<T>::min()); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    min(const Vector<T, N, Container> & a) noexcept
    { return reduce(static_cast<const T&(*)(const T&, const T&)>(std::min), a, std::numeric_limits<T>::max()); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    max(const Vector<T, N, Container1>& a,
        const Vector<T, N, Container2>& b) noexcept
    { return transform(static_cast<const T&(*)(const T&, const T&)>(std::max), a, b); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2>
    constexpr auto
    min(const Vector<T, N, Container1>& a,
        const Vector<T, N, Container2>& b) noexcept
    { return transform(static_cast<const T&(*)(const T&, const T&)>(std::min), a, b); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container1,
			  template <typename, size_t> class Container2,
              typename ULP>
    constexpr auto
    almost_equal(const Vector<T, N, Container1>& va,
                 const Vector<T, N, Container2>& vb,
                 const ULP ulp)
    { return transform([ulp](const T a, const T b){ return math::almost_equal(a, b, ulp); }, va, vb); }

    template <typename T, size_t N, typename ULP,
    	      template <typename, size_t> class Container1>
    constexpr auto
    almost_equal(const Vector<T, N, Container1>& va,
                 const T b,
                 const ULP ulp)
    { return transform([ulp, b](const T a){ return math::almost_equal(a, b, ulp); }, va); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    fit(const Vector<T, N, Container>& v,
        const T minval,
        const T maxval) noexcept
    { return transform([minval, maxval](const T& val){ return math::fit(minval, maxval, val); }, v); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    abs(const Vector<T, N, Container>& v) noexcept
    { return transform(static_cast<T(*)(T)>(math::abs), v); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    clamp(const Vector<T, N, Container>& v,
          const T minval,
          const T maxval) noexcept
    { return transform([&](const T val){ return std::clamp(val, minval, maxval); }, v); }

    template <typename T, size_t N,
    	      template <typename, size_t> class Container>
    constexpr auto
    roll(const Vector<T, N, Container>& v,
         const size_t shift) noexcept
    {
        Vector<T, N> ret{};

        for (size_t i = 0; i < N; ++i)
            ret[i] = v[(i - shift) % N];

        return ret;
    }
}

namespace std
{
	template <typename T, size_t N,
			  template <typename, size_t> typename Container>
	class tuple_size<yapt::Vector<T, N, Container>>
    {
    public:
        static const constexpr size_t value = N;
    };

	template<size_t I, typename T, size_t N,
			 template <typename, size_t> typename Container>
    class tuple_element<I, yapt::Vector<T, N, Container>>
	{
    public:
        using type = T;
    };

	template<size_t I, typename T, size_t N,
			 template <typename, size_t> typename Container>
    constexpr decltype(auto)
    get(const yapt::Vector<T, N, Container>& vec) noexcept
    { return vec.template get<I>(); }
}
