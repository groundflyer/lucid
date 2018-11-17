#include "property_test.hpp"

#include <base/vector.hpp>
#include <base/arrayview.hpp>
#include <base/rng.hpp>

#include <string>
#include <limits>
#include <numeric>

using namespace std;
using namespace yapt;

template <typename T>
RandomDistribution full_dist(numeric_limits<T>::lowest(), numeric_limits<T>::max());

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

    auto log_ok = spdlog::stdout_color_st("ok");
    auto log_fail = spdlog::stderr_color_st("fail");
    auto log_debug = spdlog::stdout_logger_st("debug");
    log_ok->set_pattern("%v: %^OK%$");
    log_fail->set_pattern("%v: %^FAIL%$");
    log_debug->set_pattern("%v");

    ret += test_property("Vector<float, N>(float)",
                         [](const float feed) { return Vector<float, N>(feed); },
                         [](const Vector<float, N> property, const float feed) { return any(property != feed); },
                         [&](){ return float_dist(g); },
                         num_tests);

    ret += test_property("Vector<float, N>(int)",
                         [](const int feed) { return Vector<float, N>(feed); },
                         [](auto&& property, auto&& feed) { return any(property != static_cast<float>(feed)); },
                         [&](){ return int_dist(g); },
                         num_tests);

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
