// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/argparse.hpp>

#include <initializer_list>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace lucid;
using namespace lucid::argparse;


struct
ArgvMock
{
    std::vector<std::string> words;
    std::vector<char*> pointers;

    void
    update_argv()
    {
        std::generate_n(std::back_inserter(pointers), words.size(), [iter=words.cbegin()]() mutable { return const_cast<char*>((iter++)->data()); });
    }

    explicit
    ArgvMock(std::initializer_list<std::string_view> _words) : words(_words.begin(), _words.end())
    {
        update_argv();
    }

    explicit
    ArgvMock(const std::vector<std::string>& _words) : words(_words)
    {
        update_argv();
    }

    char**
    argv()
    {
        return pointers.data();
    }

    int
    argc()
    {
        return pointers.size();
    }
};


constexpr tuple options{option<'a'>(identity{}, "default foo", "foo", "doc for foo", "FOO"),
                        flag<'f'>(false, "flag", "doc for flag"),
                        option<'d', 2ul>(identity{}, {"1111", "2222"}, "two-val", "doc for two-val", {"2val1", "2val2"}),
                        option_list<'m'>(identity{}, "multi_val", "doc for multi_val", "VAL")};

static_assert(!keywords_have_space(options));

constexpr tuple positionals{positional<2ul>(identity{}, "doc for 1st positional", {"pos11", "pos12"}),
                            positional(identity{}, "doc for 2nd positional", "pos2")};

static inline Logger logger(Logger::DEBUG);

int main()
{
    ArgvMock mock({"argtest", "-a", "foo value", "-d", "d1", "d2", "-m", "mval1", "mval2", "mval3", "-f", "1posval1", "1posval2", "2posval"});
    ArgsRange args(mock.argc(), mock.argv());
    try
    {
        const auto results = parse(options, positionals, args);
        const auto [f1, f2] = results.get_opt<'d'>();
        auto multi_range = results.get_opt<'m'>();
        const auto [p1, p2] = results.get_pos<0>();
        const auto p3 = results.get_pos<1>();
        logger.debug("a = {}", results.get_opt<'a'>());
        logger.debug("f = {}", results.get_opt<'f'>());
        logger.debug("d multival = {}, {}", f1, f2);
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
