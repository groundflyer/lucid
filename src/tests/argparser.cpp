// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/tuple.hpp>
#include <utils/logging.hpp>

#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <range/v3/view/facade.hpp>
#include <range/v3/algorithm/for_each.hpp>


namespace lucid::argparse
{
class ArgRange : public ranges::view_facade<ArgRange>
{
    friend ranges::range_access;

    int current = 0;
    int _argc = 0;
    char **_argv = nullptr;

public:
    ArgRange() = default;

    ArgRange(int argc, char* argv[]) : _argc(argc), _argv(argv) {}

    std::string_view
    read() const
    {
        return _argv[current];
    }

    bool
    equal(ranges::default_sentinel_t) const
    {
        return current == _argc;
    }

    void
    next()
    {
        ++current;
    }
};

template <char Key, typename Converter>
struct Option
{
    using Value = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    Value value;
    std::size_t nargs;
    Converter converter;
    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr
    Option(const Value& _value, const std::size_t _nargs, Converter _converter, const std::string_view _keyword, std::string_view _doc, const std::string_view _var) noexcept :
        value(_value), nargs(_nargs), converter(_converter), keyword(_keyword), doc(_doc), var(_var)
    {}
};

struct KeyException
{
    char value;
};

struct KeywordException
{
    std::string_view value;
};

template <bool Value>
struct OptionSwitcher
{
    template <typename ... Args>
    constexpr bool
    operator()(Args...) const noexcept
    {
        return Value;
    }
};

namespace detail
{
template <typename _Option>
struct extract_key;

template <char Key, typename _>
struct extract_key<Option<Key, _>>
{
    static constexpr char value = Key;
};

template <char Key, std::size_t Idx, typename Options>
struct key_index_impl;

template <char Key, std::size_t Idx, typename _Option, typename ... Rest>
struct key_index_impl<Key, Idx, std::tuple<_Option, Rest...>>
{
    static constexpr std::size_t value = (Key == extract_key<_Option>::value) ? Idx : key_index_impl<Key, Idx + 1, std::tuple<Rest...>>::value;
};

template <typename ... Options>
struct has_repeating_impl;

template <typename First, typename ... Rest>
struct has_repeating_impl<First, Rest...>
{
    static constexpr bool value = (false || ... || (extract_key<First>::value == extract_key<Rest>::value)) || has_repeating_impl<Rest...>::value;
};

template <typename Last>
struct has_repeating_impl<Last>
{
    static constexpr bool value = false;
};

template <std::size_t Idx, typename FirstOption, typename ... RestOptions>
constexpr std::size_t
tokenize_impl(const char key)
{
    if (key == extract_key<FirstOption>::value)
        return Idx;

    if constexpr (sizeof...(RestOptions) > 0ul)
        return tokenize_impl<Idx + 1, RestOptions...>(key);

    throw KeyException{key};
}

template <std::size_t Idx, typename ... Options>
constexpr std::size_t
tokenize_impl(const std::string_view keyword, const std::tuple<Options...>& options)
{
    if (keyword == std::get<Idx>(options).keyword)
        return Idx;

    if constexpr (Idx < sizeof...(Options) - 1)
        return tokenize_impl<Idx+1>(keyword, options);

    throw KeywordException{keyword};
}
}

template <char Key, typename Options>
struct key_index;

template <char Key, typename ... Options>
struct key_index<Key, std::tuple<Options...>>
{
    static constexpr std::size_t value = detail::key_index_impl<Key, 0ul, std::tuple<Options...>>::value;
};

template <typename ... Options>
constexpr std::size_t
tokenize(const char key, const std::tuple<Options...>)
{
    return detail::tokenize_impl<0ul, Options...>(key);
}

template <typename ... Options>
constexpr std::size_t
tokenize(const std::string_view keyword, const std::tuple<Options...>& options)
{
    return detail::tokenize_impl<0ul>(keyword, options);
}

template <typename Options>
struct has_repeating;

template <typename ... Options>
struct has_repeating<std::tuple<Options...>>
{
    static constexpr bool value = detail::has_repeating_impl<Options...>::value;
};


template <typename ... Options>
constexpr auto
make_values(const std::tuple<Options...>& options) noexcept
{
    return std::apply([](const Options&... option){ return std::tuple{option.value...}; }, options);
}

static inline Logger logger(Logger::DEBUG);

template <typename ... Options>
auto
parse(const std::tuple<Options...>& options, ArgRange args)
{
    auto values = make_values(options);

    enum class WordType
        {
         KEYCHAR,
         KEYWORD,
         VALWORD
        };

    while(!args.equal(ranges::default_sentinel))
    {
        std::string_view word = args.read();

        logger.debug("checking word {}", word);
        WordType word_type = WordType::VALWORD;

        auto word_iter = word.cbegin();
        while (*word_iter == '-')
        {
            if (word_type != WordType::KEYCHAR)
                word_type = WordType::KEYCHAR;
            else
                word_type = WordType::KEYWORD;

            ++word_iter;
        }

        auto visitor = [](auto& value){ value = "test"; };
        auto update_values = [&](const auto& key)
                             {
                                 const std::size_t token = tokenize(key, options);
                                 visit(token, visitor, values);                             
                             };

        switch (word_type)
        {
        case WordType::KEYCHAR:
            {
                const char key = *word_iter;
                logger.debug("Found key: {}", key);
                update_values(key);
                break;
            }
        case WordType::KEYWORD:
            {
                auto keyword = word.substr(2);
                logger.debug("Found keyword {}", keyword);
                update_values(keyword);
                break;
            }
        case WordType::VALWORD:
            {
                logger.debug("Found value {}", word);
                break;
            }
        }

        args.next();
    }

    return values;
}

} // namespace lucid::argparse

using namespace std;
using namespace lucid;
using namespace lucid::argparse;

constexpr tuple options{Option<'a', identity>("foo", 1, identity{}, "foo", "", ""),
                        Option<'b', identity>("bar", 1, identity{}, "bar", "", ""),
                        Option<'c', identity>("foo2", 1, identity{}, "foo2", "", "")};

static_assert(!has_repeating<decay_t<decltype(options)>>::value);

int main(int argc, char *argv[])
{
    ArgRange args(argc, argv);
    args.next();
    // ranges::for_each(args, [](std::string_view arg){ logger.debug("found arg: {}", arg); });
    try
    {
        parse(options, args);
    }
    catch (const KeyException& ex)
    {
        logger.critical("Unknown key: {}", ex.value);
    }
    catch (const KeywordException& ex)
    {
        logger.critical("Unknown keyword: {}", ex.value);
    }
    return 0;
}
