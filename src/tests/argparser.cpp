// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/argparse.hpp>

#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>

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
        pointers.clear();
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


constexpr tuple options{option<'a'>(identity{}, "default foo", "foo", "long doc for foo lore ipsum hule bly", "FOO"),
                        flag<'f'>(false, "flag", "doc for flag bla bla bla foo bar"),
                        option<'d', 2ul>(identity{}, {"1111", "2222"}, "two-val", "doc for two-val here we have an array fo string views of size two", {"2val1", "2val2"}),
                        option_list<'m'>(identity{}, "multi_val", "doc for unlimited values", "VAL")};

static_assert(!keywords_have_space(options));

constexpr tuple positionals{positional<2ul>(identity{}, "doc for 1st positional", {"pos11", "pos12"}),
                            positional(identity{}, "doc for 2nd positional", "pos2")};

/// @test Passing correct arguments to argument parser.
bool
no_error_check() noexcept
{
    ArgvMock  mock({"argtest",
                   "-a",
                   "foo value",
                   "-d",
                   "d1",
                   "d2",
                   "-m",
                   "mval1",
                   "mval2",
                   "mval3",
                   "-f",
                   "1posval1",
                   "1posval2",
                   "2posval"});
    ArgsRange args(mock.argc(), mock.argv());
    bool      ret = false;
    parse(options, positionals, args, [&ret](const auto&) { ret = true; });
    return ret;
}

/// @test Passing wrong arguments (no required positionals) to argument parser.
bool
error_check() noexcept
{
    ArgvMock mock({"argtest"});
    ArgsRange args(mock.argc(), mock.argv());
    bool ret = true;
    parse(options, positionals, args, [&ret](const auto&){ ret = false; });
    return ret;
}

int main()
{
    int ret = 0;
    ret += no_error_check();
    ret += error_check();
    show_help("lucid", options, positionals);
    return ret;
}
