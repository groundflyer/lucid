// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/argparse.hpp>

#include <cstdlib>

using namespace std;
using namespace lucid;
using namespace lucid::argparse;

struct
toint
{
    int
    operator()(std::string_view word) const noexcept
    {
        return std::atoi(word.data());
    }
};

constexpr tuple options{option<'a'>(identity{}, "default foo", "foo", "doc for foo", "FOO"),
                        flag<'f'>(false, "flag", "doc for flag"),
                        option<'d', 2ul>([](std::string_view arg){ return atoi(arg.data()); }, {1111, 2222}, "two-val", "doc for two-val", {"2val1", "2val2"}),
                        option_list<'m'>(toint{}, "multi_val", "doc for multi_val", "VAL")};

static_assert(!keywords_have_space(options));

constexpr tuple positionals{positional<2ul>(identity{}, "doc for 1st positional", {"pos11", "pos12"}),
                            positional(identity{}, "doc for 2nd positional", "pos2")};

static inline Logger logger(Logger::DEBUG);

int main(int argc, char *argv[])
{
    ArgsRange args(argc, argv);
    try
    {
        const auto results = parse(options, positionals, args);
        const auto [f1, f2] = results.get_opt<'d'>();
        auto multi_range = results.get_opt<'m'>();
        const auto [p1, p2] = results.get_pos<0>();
        const auto p3 = results.get_pos<1>();
        logger.debug("a = {}", results.get_opt<'a'>());
        logger.debug("d = {}", results.get_opt<'f'>());
        logger.debug("multival = {}, {}", f1, f2);
        logger.debug("multirange values:");
        ranges::for_each(multi_range, [&](const auto& val){ logger.debug("{}", val); });
        logger.debug("positionals: {}, {}, {}", p1, p2, p3);
    }
    catch (const KeyException<char>& ex)
    {
        logger.critical("Unknown key: {}", ex.value);
    }
    catch (const KeyException<std::string_view>& ex)
    {
        logger.critical("Unknown keyword: {}", ex.value);
    }
    return 0;
}
