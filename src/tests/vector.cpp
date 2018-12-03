#include "property_test.hpp"

#include <base/utils.hpp>
#include <base/vector.hpp>
#include <base/arrayview.hpp>
#include <base/rng.hpp>

#include <string>
#include <limits>
#include <numeric>
#include <sstream>
#include <functional>

using namespace std;
using namespace yapt;
using namespace fmt::literals;

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

template <typename T, size_t N>
auto get_typeinfo_string(array<T, N>) noexcept
{
    stringstream ss;
    auto typestring = get_typeinfo_string(T{});
    for(size_t i = 0; i < N - 1; ++i)
        ss << typestring << ", ";
    ss << typestring;
    return ss.str();
}

template <size_t I, typename A, typename B>
bool
not_equal(A&& a, B&& b)
{ return get<I>(forward<A>(a)) != get<I>(forward<B>(b)); }

template <typename A, typename B, size_t... I>
constexpr bool
assert_tuples(A&& a, B&& b, index_sequence<I...>)
{ return (false || ... || not_equal<I>(forward<A>(a), forward<B>(b))); }

template <typename T, size_t N, typename RandomEngine>
auto
test_t_n(RandomEngine& g, const size_t num_tests) noexcept
{
    using Vec = Vector<T, N>;
    const auto t_typestring = get_typeinfo_string(T{});
    const auto vec_typestring = "Vector<{}, {}>"_format(t_typestring, N);
    RandomDistribution<T> dist(T{-10000}, T{10000});
    unsigned ret = 0;

    ret += test_property("{}({})"_format(vec_typestring, t_typestring),
                         [&](){ return dist(g); },
                         [](const auto feed) { return Vec(feed); },
                         [](const auto property, const auto feed) { return any(property != feed); },
                         num_tests);

    auto argen = [&]() { return dist.template operator()<N>(g); };
    auto arass = [](const Vec property, const array<T, N> feed) { return assert_tuples(property, feed, make_index_sequence<N>{}); };

    ret += test_property("{}({})"_format(vec_typestring, get_typeinfo_string(array<T, N>{})),
                         argen,
                         [](const array<T, N> feed){ return construct<Vec>(feed); },
                         arass,
                         num_tests);

    ret += test_property("{}(array<{}, {}>)"_format(vec_typestring, t_typestring, N),
                         argen,
                         [](const array<T, N> feed) { return Vec(feed); },
                         arass,
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

    // distribution to generate divizor that is guaranteed to be non-equal zero
    RandomDistribution<T> divdist(T{1}, T{10000});

    // generate random sign
    auto signgen = [&, bdist = RandomDistribution<bool>(0.5)]() mutable
                   { return static_cast<T>(math::minus_one_pow(bdist(g))); };

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
                             return any((property - val) != vec);
                         },
                         num_tests);

    ret += test_property("{} */ {}"_format(vec_typestring, t_typestring),
                         [&]() { return pair(Vec(dist.template operator()<N>(g)), divdist(g) * signgen()); },
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

    ret += test_property("{} +- {}"_format(vec_typestring, vec_typestring),
                         [&]() { return pair(Vec(dist.template operator()<N>(g)), Vec(dist.template operator()<N>(g))); },
                         [](const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return vec1 + vec2;
                         },
                         [](const auto property, const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return any((property - vec2) != vec1);
                         },
                         num_tests);

    ret += test_property("{0} */ {0}"_format(vec_typestring),
                         [&]() { return pair(Vec(dist.template operator()<N>(g)), Vec(divdist.template operator()<N>(g)) * signgen()); },
                         [](const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return vec1 * vec2;
                         },
                         [&](const auto property, const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             if constexpr(is_floating_point_v<T>)
                                 return any(!almost_equal(property / vec2, vec1));
                             else
                                 return any((property / vec2) != vec1);
                         },
                         num_tests);

    return ret;
}

template <size_t N, typename RandomEngine, typename... Ts>
auto
test_t(RandomEngine& g, size_t num_tests, typelist<Ts...>) noexcept
{ return (0u + ... + test_t_n<Ts, N>(g, num_tests)); }

using ArithmeticTypes = typelist<int, long, float, double>;

template <typename RandomEngine, size_t... Ns>
auto
test_n(RandomEngine& g, const size_t num_tests, index_sequence<Ns...>) noexcept
{ return (0u + ... + test_t<Ns + 2>(g, num_tests, ArithmeticTypes{})); }


template <size_t N, typename RandomEngine>
auto
__boolean_test(RandomEngine& g, const size_t num_tests) noexcept
{
    RandomDistribution<bool> dist(0.5);
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

static const constexpr size_t MaxN = 4;
using Indicies = make_index_sequence<MaxN - 1>;

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? stoul(argv[1]) : 1000000;

    int ret = 0;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    Indicies idxs;

    ret += test_n(g, num_tests, idxs);
    ret += boolean_test(g, num_tests, idxs);

    return ret;
}
