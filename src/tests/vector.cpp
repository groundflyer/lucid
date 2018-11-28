#include "property_test.hpp"

#include <base/utils.hpp>
#include <base/vector.hpp>
#include <base/arrayview.hpp>
#include <base/rng.hpp>

#include <string>
#include <limits>
#include <numeric>

using namespace std;
using namespace yapt;
using namespace fmt::literals;


template <typename T>
RandomDistribution<T>&
get_dist() noexcept
{
    static const constexpr T a = std::is_floating_point_v<T> ? -static_cast<T>(10000) : std::numeric_limits<T>::lowest();
    static const constexpr T b = std::is_floating_point_v<T> ? static_cast<T>(10000) : std::numeric_limits<T>::max();
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
constexpr auto get_typeinfo_string(std::tuple<Ts...>) noexcept
{ return "{}"_format(std::tuple(get_typeinfo_string(Ts{})...)); }

template <size_t TupleSize, size_t Start,
          typename RandomEngine, typename Typelist, size_t... I>
constexpr auto
_random_cycled_tuple(RandomEngine& g, Typelist, std::index_sequence<I...>)
{ return std::tuple(get_dist<typename typelist_element<(Start + I) % TupleSize, Typelist>::type>()(g)...); }

template <size_t TupleSize, size_t Start,
          typename RandomEngine, typename... Ts>
constexpr auto
random_cycled_tuple(RandomEngine& g, typelist<Ts...> tl)
{ return _random_cycled_tuple<sizeof...(Ts), Start>(g, tl, std::make_index_sequence<TupleSize>{}); }

template <typename Vec, typename T, typename RandomEngine>
constexpr auto
_single_value_constructor_test(std::string_view type_string,
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
single_value_constructor_test(std::string_view type_string,
                              RandomEngine& g,
                              const size_t num_tests,
                              typelist<Ts...>) noexcept
{ return (0 + ... + _single_value_constructor_test<Vec, Ts>(type_string, g, num_tests)); }

template <size_t I, typename A, typename B>
constexpr bool
not_equal(const A& a, const B& b)
{ return std::get<I>(a) != static_cast<std::decay_t<decltype(std::get<I>(a))>>(std::get<I>(b)); }

template <typename A, typename B, size_t... I>
constexpr bool
assert_tuples(const A& a, const B& b, std::index_sequence<I...>)
{ return (false || ... || not_equal<I>(a, b)); }

template <typename Vec, size_t N, size_t Shift, typename RandomEngine, typename GenTypes>
constexpr auto
__multi_value_constructor_test(std::string_view type_string,
                              RandomEngine& g,
                              const size_t num_tests,
                              GenTypes tl)
{
    auto tg = [&g, tl](){ return random_cycled_tuple<N, Shift>(g, tl); };
    return test_property("{}{}"_format(type_string, get_typeinfo_string(tg())),
                         tg,
                         [](const auto feed){ return construct<Vec>(feed); },
                         [](const Vec property, const auto feed) { return assert_tuples(property, feed, std::make_index_sequence<N>{}); },
                         num_tests);
}

template <typename Vec, size_t N, typename RandomEngine, typename GenTypes, size_t... I>
constexpr auto
_multi_value_constructor_test(std::string_view type_string,
                             RandomEngine& g,
                             const size_t num_tests,
                             GenTypes tl,
                             std::index_sequence<I...>)
{ return (0u + ... + __multi_value_constructor_test<Vec, N, I>(type_string, g, num_tests, tl)); }

template <typename Vec, size_t N, typename RandomEngine, typename... GenTypes>
constexpr auto
multi_value_constructor_test(std::string_view type_string,
                             RandomEngine& g,
                             const size_t num_tests,
                             typelist<GenTypes...> gen_types)
{ return _multi_value_constructor_test<Vec, N>(type_string, g, num_tests, gen_types, std::make_index_sequence<sizeof...(GenTypes) - N + 1>{}); }

using test_types = typelist<int, bool, char, long, float, double, size_t, long double, unsigned, unsigned char, unsigned short>;

template <typename T, size_t N, typename RandomEngine>
auto
__constructor_test(RandomEngine& g, const size_t num_tests) noexcept
{
    using Vec = Vector<T, N>;
    const auto t_ts = get_typeinfo_string(T{});
    const auto vec_ts = "Vector<{}, {}>"_format(t_ts, N);

    unsigned ret = 0;

    auto tts = test_types{};

    ret += single_value_constructor_test<Vec>(vec_ts, g, num_tests, tts);

    ret += multi_value_constructor_test<Vec, N>(vec_ts, g, num_tests, tts);

    ret += test_property("{}(std::array<{}, {}>)"_format(vec_ts, get_typeinfo_string(T{}), N),
                         [&g]() { return get_dist<T>().template operator()<N>(g); },
                         [](const std::array<T, N> feed) { return Vec(feed); },
                         [](const Vec property, const std::array<T, N> feed) { return assert_tuples(property, feed, std::make_index_sequence<N>{}); },
                         num_tests);

    if constexpr (N > 2)
    {
        decltype(auto) dist_t = get_dist<T>();
        const constexpr auto N1 = N - 1;
        ret += test_property("{0}({1}, Vector<{1}, {2}>)"_format(vec_ts, t_ts, N1),
                             [&]() { return std::tuple(dist_t(g), Vector<T, N1>(dist_t.template operator()<N1>(g))); },
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

static const constexpr size_t MaxN = 4;

template <typename T, typename RandomEngine, size_t... Ns>
auto
_constructor_test(RandomEngine& g, size_t num_tests, std::index_sequence<Ns...>) noexcept
{ return (0 + ... + __constructor_test<T, Ns + 2>(g, num_tests)); }

template <typename RandomEngine, typename... Ts>
auto
constructor_test(RandomEngine& g, size_t num_tests, typelist<Ts...>) noexcept
{ return (0 + ... + _constructor_test<Ts>(g, num_tests, std::make_index_sequence<MaxN - 1>{})); }

int
main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? std::stoll(argv[1]) : 1000000;

    int ret = 0;

    random_device rd;
    default_random_engine g(rd());

    init_log();

    ret += constructor_test(g, num_tests, test_types{});

    // // ret += test_property("any(Vector<bool, N>)",
    // //                      [](const Vector<bool, N>& feed) { return any(feed); },
    // //                      [](const bool property, const Vector<bool, N>& feed)
    // //                      { return property != apply([](auto... vals) { return (true && ... && vals); }, feed); },
    // //                      [&]() { return Vector(get_dist<bool>().template operator()<N>(g)); },
    // //                      num_tests);

    // ret += test_property("Vector<float, N> +- float",
    //                      [](const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return vec + val;
    //                      },
    //                      [](const auto property, const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return any(property - val != vec);
    //                      },
    //                      [&]() { return pair(Vector(float_dist.operator()<N>(g)), float_dist(g)); },
    //                      num_tests);

    // ret += test_property("Vector<float, N> +- int",
    //                      [](const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return vec + val;
    //                      },
    //                      [](const auto property, const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return any(property - val != vec);
    //                      },
    //                      [&]() { return pair(Vector(float_dist.operator()<N>(g)), int_dist(g)); },
    //                      num_tests);

    // ret += test_property("Vector<float, N> +- Vector<float, N>",
    //                      [](const auto feed)
    //                      {
    //                          const auto& [vec1, vec2] = feed;
    //                          return vec1 + vec2;
    //                      },
    //                      [](const auto property, const auto feed)
    //                      {
    //                          const auto& [vec1, vec2] = feed;
    //                          return any(property - vec1 != vec2);
    //                      },
    //                      [&]() { return pair(Vector(float_dist.operator()<N>(g)), Vector(float_dist.operator()<N>(g))); },
    //                      num_tests);

    // ret += test_property("Vector<float, N> */ int",
    //                      [](const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return vec * val;
    //                      },
    //                      [](const auto property, const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return any(!almost_equal(property / val, vec));
    //                      },
    //                      [&]() { return pair(Vector(float_dist.operator()<N>(g)), int_dist(g)); },
    //                      num_tests);

    // ret += test_property("Vector<float, N> */ float",
    //                      [](const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return vec * val;
    //                      },
    //                      [](const auto property, const auto feed)
    //                      {
    //                          const auto& [vec, val] = feed;
    //                          return any(!almost_equal(property / val, vec));
    //                      },
    //                      [&]() { return pair(Vector(float_dist.operator()<N>(g)), float_dist(g)); },
    //                      num_tests);

    // ret += test_property("Vector<float, N> */ Vector<float, N>",
    //                      [](const auto feed)
    //                      {
    //                          const auto& [vec1, vec2] = feed;
    //                          return vec1 * vec2;
    //                      },
    //                      [](const auto property, const auto feed)
    //                      {
    //                          const auto& [vec1, vec2] = feed;
    //                          return any(!almost_equal(property / vec2, vec1));
    //                      },
    //                      [&]() { return pair(Vector(float_dist.operator()<N>(g)), float_dist(g)); },
    //                      num_tests);

    return ret;
}
