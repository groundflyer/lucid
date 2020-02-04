// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/tuple.hpp>
#include <utils/logging.hpp>

#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/facade.hpp>
#include <range/v3/view/slice.hpp>


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

    ArgRange(int argc, char* argv[]) noexcept : _argc(argc), _argv(argv) {}

    std::string_view
    read() const noexcept
    {
        return _argv[current];
    }

    bool
    equal(ranges::default_sentinel_t) const noexcept
    {
        return current == _argc;
    }

    void
    next() noexcept
    {
        ++current;
    }

    void
    advance(const std::ptrdiff_t n) noexcept
    {
        current += n;
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


template <typename ... Pairs>
constexpr auto
make_values(const std::tuple<Pairs...>& converters) noexcept
{
    return std::apply([](const Pairs&... conv_val){ return std::tuple{conv_val.second...}; }, converters);
}

template <typename ... Options>
constexpr auto
make_converters(const std::tuple<Options...>& options) noexcept
{
    return std::apply([](const Options&... option){ return std::tuple{std::pair{option.converter, option.value}...}; }, options);
}

static inline Logger logger(Logger::DEBUG);

std::size_t
values_extent(ArgRange args) noexcept
{
    std::size_t ret = 0ul;

    while(!args.equal(ranges::default_sentinel) && args.read()[0] != '-')
    {
        ++ret;
        args.next();
    }

    return ret;
}

template <typename ... Options>
auto
parse(const std::tuple<Options...>& options, ArgRange args)
{
    auto converters = make_converters(options);

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

        auto update_values = [&](const auto& key, std::string_view value_word)
                             {
                                 const std::size_t token = tokenize(key, options);
                                 logger.debug("setting {} to {}", key, value_word);
                                 visit(token, [&value_word](auto& pair){ pair.second = pair.first(value_word); }, converters);
                             };

        switch (word_type)
        {
        case WordType::KEYCHAR:
            {
                const char key = *word_iter;
                logger.debug("Found key: {}", key);
                args.next();
                update_values(key, args.read());
                break;
            }
        case WordType::KEYWORD:
            {
                auto keyword = word.substr(2);
                logger.debug("Found keyword {}", keyword);
                args.next();
                update_values(keyword, args.read());
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

    return make_values(converters);
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
    logger.debug("Values extent {}", values_extent(args));
    // ranges::for_each(ranges::views::slice(args, 1, 4), [](std::string_view arg){ logger.debug("found arg: {}", arg); });
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
