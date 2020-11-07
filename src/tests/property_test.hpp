// -*- C++ -*-
// property_test.hpp
//

/// @file
/// Simple property-based testing.

#pragma once

#include <utils/functional.hpp>

#include <fmt/format.h>

#include <string_view>
#include <utility>

/// @brief Test a @p property of @p testing function.
/// @param n Number of runs.
/// @param threshold Ratio of runs allowed to fail.
/// @param property_name Label to display.
/// @param generator Callable that generates a unique value each time its called.
/// @param testing Function that takes the output of @p generator and returns some value.
/// @param property Function that takes the outputs of @p generator and @p testing and asserts them
/// returning bool.
/// @returns true if test failed otherwise false.
///
/// Simplified approach to property-based testing.
///
/// Unlike classic unit testing, in property-based testing
/// we test functions many times against various values.
/// Properties are invariant, so it makes testing more robust.
///
/// First, we need to find a property of the testing function,
/// or we can just use inverse functions, e.g. minus and plus thus
/// testing two functions at the time.
///
/// Then we define our @p generator.
/// It outputs random values in a valid range
/// acceptable by @p testing function.
///
/// @p testing is the function we'd like to test.
///
/// If @p testing accepts more then one value,
/// @p generator should return either a tuple or a pair.
/// @p test_property will decide automatically how to call @p testing.
///
/// @p property checks if the property is preserved.
/// It accepts the output of @p testing as the first argument
/// and the output of @p generator as the second argument.
/// In case of fail it returns true, otherwise false.
///
/// The test is probablistic: if the portion of failed tests
/// is less than @p threshold, test passed.
template <typename Testing, typename Property, typename Generator>
bool
test_property(const std::size_t n,
              const double      threshold,
              std::string_view  property_name,
              Generator&&       generator,
              Testing&&         testing,
              Property&&        property)
{
    std::size_t sum = 0;
    for(std::size_t i = 0; i < n; ++i)
    {
        const auto feed   = generator();
        const bool result = property(lucid::maybe_apply(testing, feed), feed);
        sum += static_cast<size_t>(result);
    }

    const double error = static_cast<double>(sum) / static_cast<double>(n);

    const bool                  ret     = error > threshold;
    static const constexpr char RED[]   = "\033[31m";
    static const constexpr char GREEN[] = "\033[32m";
    static const constexpr char RESET[] = "\033[0m";
    if(ret)
        fmt::print("{}FAIL{} {:.3f}% {}\n", RED, RESET, error * 100, property_name);
    else
        fmt::print("{}OK{} {:.3f}% {}\n", GREEN, RESET, error * 100, property_name);

    return ret > 0;
}
