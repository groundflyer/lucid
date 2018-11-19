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
    static const constexpr T bound = static_cast<T>(10000);
    static RandomDistribution<T> dist(is_unsigned_v<T> ? 0 : -bound, bound);
    return dist;
}

template <>
RandomDistribution<bool>&
get_dist() noexcept
{
    static RandomDistribution<bool> dist(0.5);
    return dist;
}

template <typename T>
struct typeinfo;

template <>
struct typeinfo<int>
{ static const constexpr char string[] = "int"; };

template <>
struct typeinfo<bool>
{ static const constexpr char string[] = "bool"; };

template <>
struct typeinfo<char>
{ static const constexpr char string[] = "char"; };

template <>
struct typeinfo<long>
{ static const constexpr char string[] = "long"; };

template <>
struct typeinfo<float>
{ static const constexpr char string[] = "float"; };

template <>
struct typeinfo<double>
{ static const constexpr char string[] = "double"; };

template <>
struct typeinfo<size_t>
{ static const constexpr char string[] = "size_t"; };

template <>
struct typeinfo<long double>
{ static const constexpr char string[] = "long double"; };

template <>
struct typeinfo<unsigned int>
{ static const constexpr char string[] = "unsigned int"; };

template <>
struct typeinfo<unsigned char>
{ static const constexpr char string[] = "unsigned char"; };

template <typename Vec, typename T, typename RandomEngine>
auto
_single_value_constructor_test(std::string_view type_string,
                               RandomEngine& g,
                               size_t num_tests) noexcept
{
    return test_property("{}({})"_format(type_string, typeinfo<T>::string),
                         [](const auto feed) { return Vec(feed); },
                         [](const auto property, const auto feed) { return any(property != feed); },
                         [&](){ return get_dist<T>()(g); },
                         num_tests);
}

template <typename Vec, typename RandomEngine, typename... Ts>
auto
single_value_constructor_test(std::string_view type_string,
                              RandomEngine& g,
                              size_t num_tests,
                              typelist<Ts...>) noexcept
{ return (0 + ... + _single_value_constructor_test<Vec, Ts>(type_string, g, num_tests)); }

using test_types = typelist<int, bool, char, long, float, double, size_t, long double, unsigned, unsigned char>;

template <typename T, size_t N, typename RandomEngine>
auto
vector_test(RandomEngine& g, size_t num_tests) noexcept
{
    using Vec = Vector<T, N>;
    const auto type_string = "Vector<{}, {}>"_format(typeinfo<T>::string, N);

    unsigned ret = 0;

    ret += single_value_constructor_test<Vec>(type_string, g, num_tests, test_types{});

    return ret;
}

int main(int argc, char* argv[])
{
    const size_t num_tests = argc == 2 ? std::stoll(argv[1]) : 10000;

    int ret = 0;

    random_device rd;
    default_random_engine g(rd());
    RandomDistribution float_dist(-10000.f, 10000.f);
    RandomDistribution int_dist(-10000, 10000);
    RandomDistribution unsigned_dist(0u, 10000u);
    RandomDistribution double_dist(-1e+10, 1e+10);

    const static constexpr size_t N = 4;

    init_log();

    ret += vector_test<float, 3>(g, num_tests);

    ret += test_property("Vector<float, N>(float, float, float, float)",
                         [](auto&& feed) { return construct<Vector<float, 4>>(forward<decltype(feed)>(feed)); },
                         [](auto&& property, auto&& feed)
                         {
                             return sum(property) != apply([](auto... vals) { return (0 + ... + vals); },
                                                           forward<decltype(feed)>(feed));
                         },
                         [&]() { return tuple(float_dist(g), float_dist(g), float_dist(g), float_dist(g)); },
                         num_tests);

    ret += test_property("Vector<double, N>(double, float, int, unsigned)",
                         [](auto&& feed) { return construct<Vector<double, 4>>(forward<decltype(feed)>(feed)); },
                         [](auto&& property, auto&& feed)
                         {
                             return sum(property) != apply([](auto... vals) { return (0 + ... + vals); },
                                                           forward<decltype(feed)>(feed));
                         },
                         [&]() { return tuple(double_dist(g), float_dist(g), int_dist(g), unsigned_dist(g)); },
                         num_tests);

    // ret += test_property("any(Vector<bool, N>)",
    //                      [](const Vector<bool, N> feed) { return any(feed); },
    //                      [](const bool property, const Vector<bool, N> feed)
    //                      {
    //                          return property != apply([](auto... vals) { return (true && ... && vals); },
    //                                                   feed);
    //                      },
    //                      [&]() { return Vector(bool_dist.operator()<N>(g)); },
    //                      num_tests);

    ret += test_property("Vector<float, N> +- float",
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
                         [&]() { return pair(Vector(float_dist.operator()<N>(g)), float_dist(g)); },
                         num_tests);

    ret += test_property("Vector<float, N> +- int",
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
                         [&]() { return pair(Vector(float_dist.operator()<N>(g)), int_dist(g)); },
                         num_tests);

    ret += test_property("Vector<float, N> +- Vector<float, N>",
                         [](const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return vec1 + vec2;
                         },
                         [](const auto property, const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return any(property - vec1 != vec2);
                         },
                         [&]() { return pair(Vector(float_dist.operator()<N>(g)), Vector(float_dist.operator()<N>(g))); },
                         num_tests);

    ret += test_property("Vector<float, N> */ int",
                         [](const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return vec * val;
                         },
                         [](const auto property, const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return any(!almost_equal(property / val, vec));
                         },
                         [&]() { return pair(Vector(float_dist.operator()<N>(g)), int_dist(g)); },
                         num_tests);

    ret += test_property("Vector<float, N> */ float",
                         [](const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return vec * val;
                         },
                         [](const auto property, const auto feed)
                         {
                             const auto& [vec, val] = feed;
                             return any(!almost_equal(property / val, vec));
                         },
                         [&]() { return pair(Vector(float_dist.operator()<N>(g)), float_dist(g)); },
                         num_tests);

    ret += test_property("Vector<float, N> */ Vector<float, N>",
                         [](const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return vec1 * vec2;
                         },
                         [](const auto property, const auto feed)
                         {
                             const auto& [vec1, vec2] = feed;
                             return any(!almost_equal(property / vec2, vec1));
                         },
                         [&]() { return pair(Vector(float_dist.operator()<N>(g)), float_dist(g)); },
                         num_tests);

    return ret;
}
