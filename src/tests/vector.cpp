#include "property_test.hpp"

#include <base/utils.hpp>
#include <base/vector.hpp>
#include <base/arrayview.hpp>
#include <base/rng.hpp>

#include <string>
#include <limits>
#include <numeric>
#include <functional>

using namespace std;
using namespace yapt;
using namespace fmt::literals;


template <typename T>
RandomDistribution<T>&
get_dist() noexcept
{
    static const constexpr T a = is_floating_point_v<T> ? -static_cast<T>(10000) : numeric_limits<T>::lowest();
    static const constexpr T b = is_floating_point_v<T> ? static_cast<T>(10000) : numeric_limits<T>::max();
    static RandomDistribution<T> dist(a, b);
    return dist;
}

template <>
RandomDistribution<bool>&
get_dist() noexcept
{
    static RandomDistribution<bool> dist(0.5);
    return dist;
}

#define MAKE_TYPEINFO_STRING(TYPE)                    \
    constexpr auto get_typeinfo_string(TYPE) noexcept \
    { return #TYPE; }

MAKE_TYPEINFO_STRING(int)
MAKE_TYPEINFO_STRING(bool)
MAKE_TYPEINFO_STRING(char)
MAKE_TYPEINFO_STRING(long)
MAKE_TYPEINFO_STRING(float)
MAKE_TYPEINFO_STRING(double)
MAKE_TYPEINFO_STRING(size_t)
MAKE_TYPEINFO_STRING(long double)
MAKE_TYPEINFO_STRING(unsigned int)
MAKE_TYPEINFO_STRING(unsigned char)
MAKE_TYPEINFO_STRING(unsigned short)

template <typename... Ts>
constexpr auto get_typeinfo_string(tuple<Ts...>) noexcept
{ return "{}"_format(tuple(get_typeinfo_string(Ts{})...)); }

template <size_t TupleSize, size_t Start,
          typename RandomEngine, typename Typelist, size_t... I>
auto
_random_cycled_tuple(RandomEngine& g, Typelist, index_sequence<I...>)
{ return tuple(get_dist<typename typelist_element<(Start + I) % TupleSize, Typelist>::type>()(g)...); }

template <size_t TupleSize, size_t Start,
          typename RandomEngine, typename... Ts>
auto
random_cycled_tuple(RandomEngine& g, typelist<Ts...> tl)
{ return _random_cycled_tuple<sizeof...(Ts), Start>(g, tl, make_index_sequence<TupleSize>{}); }

template <typename Vec, typename T, typename RandomEngine>
auto
_single_value_constructor_test(string_view type_string,
                               RandomEngine& g,
                               const size_t num_tests) noexcept
{
    return test_property("{}({})"_format(type_string, get_typeinfo_string(T{})),
                         [&g](){ return get_dist<T>()(g); },
                         [](const auto feed) { return Vec(feed); },
                         [](const auto property, const auto feed) { return any(property != feed); },
                         num_tests);
}

template <typename Vec, typename RandomEngine, typename... Ts>
constexpr auto
single_value_constructor_test(string_view type_string,
                              RandomEngine& g,
                              const size_t num_tests,
                              typelist<Ts...>) noexcept
{ return (0u + ... + _single_value_constructor_test<Vec, Ts>(type_string, g, num_tests)); }

template <size_t I, typename A, typename B>
bool
not_equal(A&& a, B&& b)
{ return get<I>(forward<A>(a)) != static_cast<decay_t<decltype(get<I>(forward<A>(a)))>>(get<I>(forward<B>(b))); }

template <typename A, typename B, size_t... I>
constexpr bool
assert_tuples(A&& a, B&& b, index_sequence<I...>)
{ return (false || ... || not_equal<I>(forward<A>(a), forward<B>(b))); }

template <typename Vec, size_t N, size_t Shift, typename RandomEngine, typename GenTypes>
auto
__multi_value_constructor_test(string_view type_string,
                              RandomEngine& g,
                              const size_t num_tests,
                              GenTypes tl)
{
    auto tg = [&g, tl](){ return random_cycled_tuple<N, Shift>(g, tl); };
    return test_property("{}{}"_format(type_string, get_typeinfo_string(tg())),
                         tg,
                         [](const auto feed){ return construct<Vec>(feed); },
                         [](const Vec property, const auto feed) { return assert_tuples(property, feed, make_index_sequence<N>{}); },
                         num_tests);
}

template <typename Vec, size_t N, typename RandomEngine, typename GenTypes, size_t... I>
auto
_multi_value_constructor_test(string_view type_string,
                             RandomEngine& g,
                             const size_t num_tests,
                             GenTypes tl,
                             index_sequence<I...>)
{ return (0u + ... + __multi_value_constructor_test<Vec, N, I>(type_string, g, num_tests, tl)); }

template <typename Vec, size_t N, typename RandomEngine, typename... GenTypes>
auto
multi_value_constructor_test(string_view type_string,
                             RandomEngine& g,
                             const size_t num_tests,
                             typelist<GenTypes...> gen_types)
{ return _multi_value_constructor_test<Vec, N>(type_string, g, num_tests, gen_types, make_index_sequence<sizeof...(GenTypes) - N + 1>{}); }

using FundamentalTypes = typelist<int, bool, char, long, float, double, size_t, long double, unsigned, unsigned char, unsigned short>;
using ArithmeticTypes = typelist<int, long, float, double>;

template <typename T, size_t N, typename RandomEngine>
auto
__constructor_test(RandomEngine& g, const size_t num_tests) noexcept
{
    using Vec = Vector<T, N>;
    const auto t_typestring = get_typeinfo_string(T{});
    const auto vec_typestring = "Vector<{}, {}>"_format(t_typestring, N);
    FundamentalTypes test_types;
    decltype(auto) dist = get_dist<T>();
    unsigned ret = 0;

    ret += single_value_constructor_test<Vec>(vec_typestring, g, num_tests, test_types);
    ret += multi_value_constructor_test<Vec, N>(vec_typestring, g, num_tests, test_types);
    ret += test_property("{}(array<{}, {}>)"_format(vec_typestring, t_typestring, N),
                         [&]() { return dist.template operator()<N>(g); },
                         [](const array<T, N> feed) { return Vec(feed); },
                         [](const Vec property, const array<T, N> feed) { return assert_tuples(property, feed, make_index_sequence<N>{}); },
                         num_tests);

    if constexpr (N > 2)
    {
        const constexpr auto N1 = N - 1;
        ret += test_property("{0}({1}, Vector<{1}, {2}>)"_format(vec_typestring, t_typestring, N1),
                             [&]() { return tuple(dist(g), Vector<T, N1>(dist.template operator()<N1>(g))); },
                             [](const auto feed) { return construct<Vec>(feed); },
                             [](const Vec property, const auto feed)
                             {
                                 const auto& [v0, vv] = feed;
                                 bool ret = property[0] != v0;
                                 for(size_t i = 0; i < N1; ++i)
                                     ret |= vv[i] != property[i + 1];
                                 return ret;
                             },
                             num_tests);
    }

    return ret;
}

template <size_t N, typename RandomEngine, typename... Ts>
auto
_constructor_test(RandomEngine& g, size_t num_tests, typelist<Ts...>) noexcept
{ return (0u + ... + __constructor_test<Ts, N>(g, num_tests)); }

template <typename RandomEngine, size_t... Ns>
auto
constructor_test(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + _constructor_test<Ns + 2>(g, num_tests, FundamentalTypes{})); }


template <size_t N, typename RandomEngine>
auto
__boolean_test(RandomEngine& g, const size_t num_tests) noexcept
{
    decltype(auto) dist = get_dist<bool>();
    unsigned ret = 0;

    ret += test_property("any(Vector<bool, {}>)"_format(N),
                         [&]() { return Vector(dist.template operator()<N>(g)); },
                         [](const Vector<bool, N> feed) { return any(feed); },
                         [](const bool property, const Vector<bool, N> feed)
                         {
                             return property != apply([](auto... vals) { return (false || ... || vals); },
                                                      feed.data());
                         },
                         num_tests);

    ret += test_property("all(Vector<bool, {}>)"_format(N),
                         [&]() { return Vector(dist.template operator()<N>(g)); },
                         [](const Vector<bool, N> feed) { return all(feed); },
                         [](const bool property, const Vector<bool, N> feed)
                         {
                             return property != apply([](auto... vals) { return (true && ... && vals); },
                                                      feed.data());
                         },
                         num_tests);

    return ret;
}

template <typename RandomEngine, size_t... Ns>
auto
boolean_test(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + __boolean_test<Ns + 2>(g, num_tests)); }

// template <typename T, size_t N>
// decltype(auto)
// get_comparator() noexcept
// {
//     if constexpr (is_integral_v<T>)
//         return equal_to<Vector<T, N>>();
//     else
//         return static_cast<Vector<bool, N>(*)(const Vector<T, N>&, const Vector<T, N>&, const int)>(almost_equal);
// }

template <typename T, size_t N, typename RandomEngine>
auto
__arithmetic_test(RandomEngine& g, const size_t num_tests) noexcept
{
    using Vec = Vector<T, N>;
    const auto t_typestring = get_typeinfo_string(T{});
    const auto vec_typestring = "Vector<{}, {}>"_format(t_typestring, N);
    decltype(auto) dist = get_dist<T>();
    unsigned ret = 0;

    ret += test_property("{} +- {}"_format(vec_typestring, t_typestring),
                         [&]() { return pair(Vec(dist.template operator()<N>(g)), dist(g)); },
                         [](const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return vec + val;
                         },
                         [](const auto property, const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return any(property - val != vec);
                         },
                         num_tests);

    ret += test_property("{} */ {}"_format(vec_typestring, t_typestring),
                         [&]() { return pair(Vec(dist.template operator()<N>(g)), dist(g)); },
                         [](const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return vec * val;
                         },
                         [&](const auto property, const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             if constexpr(is_floating_point_v<T>)
                                 return any(!almost_equal(property / val, vec));
                             else
                                 return any((property / val) != vec);
                         },
                         num_tests);

    return ret;
}

template <size_t N, typename RandomEngine, typename... Ts>
auto
_arithmetic_test(RandomEngine& g, size_t num_tests, typelist<Ts...>) noexcept
{ return (0u + ... + __arithmetic_test<Ts, N>(g, num_tests)); }

template <typename RandomEngine, size_t... Ns>
auto
arithmetic_test(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + _arithmetic_test<Ns + 2>(g, num_tests, ArithmeticTypes{})); }

static const constexpr size_t MaxN = 4;
using Indicies = make_index_sequence<MaxN - 1>;

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoll(argv[1]) : 1000000;

    int ret = 0;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    Indicies idxs;

    ret += constructor_test(g, num_tests, idxs);
    ret += boolean_test(g, num_tests, idxs);
    ret += arithmetic_test(g, num_tests, idxs);

    return ret;
}
