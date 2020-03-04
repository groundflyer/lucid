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
class ArgsRange : public ranges::view_facade<ArgsRange>
{
    friend ranges::range_access;

    int current = 0;
    int _argc = 0;
    char **_argv = nullptr;

public:
    ArgsRange() = default;

    ArgsRange(int argc, char* argv[]) noexcept : _argc(argc), _argv(argv) {}

    ArgsRange(ArgsRange range, int extent) noexcept : current(range.current), _argc(current + extent), _argv(range._argv) {}

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
};

struct FlagConverter
{
    struct Exception
    {
        std::string_view word;
    };

    constexpr bool
    operator()(std::string_view word) const
    {
        if (word == "yes")
            return true;

        if (word == "no")
            return false;

        throw Exception{word};
    }
};


template <typename Converter>
class Binding
{
    using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    value_type default_value;
    Converter converter;
    std::string_view word;
    bool is_set = false;

public:
    explicit constexpr
    Binding(const value_type& value) noexcept : default_value(value) {}

    void
    set(std::string_view new_word) noexcept
    {
        is_set = true;
        word = new_word;
    }

    value_type
    operator()() const
    {
        if constexpr (std::is_same_v<value_type, bool>)
            // flip default bool value or convert word if it's available
            return word.empty() ? default_value ^ is_set : converter(word);
        else
            return is_set ? converter(word) : default_value;
    }
};


template <typename Converter>
class BindingRange : public ranges::view_facade<BindingRange<Converter>>
{
    using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    friend ranges::range_access;

    Converter converter;

    value_type
    read() const noexcept
    {
        return converter(words_range.read());
    }

    bool
    equal(ranges::default_sentinel_t sentinel) const noexcept
    {
        return words_range.equal(sentinel);
    }

    void
    next() noexcept
    {
        words_range.next();
    }

public:
    BindingRange() = default;

    ArgsRange words_range;
};


template <char Key, typename Converter>
struct Option
{
    static constexpr char key = Key;
    using converter = Converter;
    using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    value_type value;
    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr
    Option(const value_type& _value, const std::string_view _keyword, const std::string_view _doc, const std::string_view _var) noexcept :
        value(_value), keyword(_keyword), doc(_doc), var(_var)
    {}

    constexpr Binding<Converter>
    binding() const noexcept
    {
        return Binding<Converter>(value);
    }
};

template <char Key>
using Flag = Option<Key, FlagConverter>;

// template <char Key, typename Converter, std::size_t min_n, std::size_t max_n>
// struct MultiOption
// {
//     static constexpr char key = Key;
//     using converter = Converter;
//     using value_type = BindingRange<Converter>;
// };

struct KeyException
{
    char value;
};

struct KeywordException
{
    std::string_view value;
};


namespace detail
{
template <char Key, std::size_t Idx, typename ... Options>
struct key_index_impl;

template <char Key, std::size_t Idx>
struct key_index_impl<Key, Idx>
{
    static constexpr std::size_t value = -1ul;
};

template <char Key, std::size_t Idx, typename _Option, typename ... Rest>
struct key_index_impl<Key, Idx, _Option, Rest...>
{
    static constexpr std::size_t value = (Key == _Option::key) ? Idx : key_index_impl<Key, Idx + 1, Rest...>::value;
};

template <typename ... Options>
struct has_repeating_impl;

template <typename First, typename ... Rest>
struct has_repeating_impl<First, Rest...>
{
    static constexpr bool value = (false || ... || (First::key == Rest::key)) || has_repeating_impl<Rest...>::value;
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
    if (key == FirstOption::key)
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
} // detail

template <char Key, typename Options>
struct key_index;

template <char Key, typename ... Options>
struct key_index<Key, std::tuple<Options...>>
{
    static const constexpr std::size_t value = detail::key_index_impl<Key, 0ul, Options...>::value;
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


template <typename Bindings, typename Options>
struct ParseResults
{
    Bindings bindings;

    constexpr
    ParseResults(const Bindings& _bindings, const Options&) : bindings(_bindings) {}

    template <char Key>
    decltype(auto)
    get() const noexcept
    {
        return std::get<key_index<Key, Options>::value>(bindings)();
    }
};


static inline Logger logger(Logger::DEBUG);

template <typename ... Options>
auto
parse(const std::tuple<Options...>& options, ArgsRange args)
{
    auto bindings = std::apply([](const Options& ... options)
                               {
                                   return std::tuple{options.binding()...};
                               }, options);

    std::size_t token = -1ul;

    while(!args.equal(ranges::default_sentinel))
    {
        std::string_view word = args.read();
        bool is_key = false;
        std::size_t pos = 0ul;

        while (word[pos] == '-' && pos <= 2u)
            ++pos;

        is_key = pos > 0;

        logger.debug("Iterating word {}", word);

        if (is_key)
            token = tokenize(word[pos], options);
        else
            visit(token, [&](auto& binding){ binding.set(word); }, bindings);

        args.next();
    }

    return ParseResults(bindings, options);
}

} // namespace lucid::argparse

using namespace std;
using namespace lucid;
using namespace lucid::argparse;

constexpr tuple options{Option<'a', identity>("foo", "foo", "", ""),
                        Option<'b', identity>("bar", "bar", "", ""),
                        Option<'c', identity>("foo2", "foo2", "", ""),
                        Flag<'d'>(false, "flag", "", "")};

static_assert(!has_repeating<decay_t<decltype(options)>>::value);

int main(int argc, char *argv[])
{
    ArgsRange args(argc, argv);
    args.next();
    // ranges::for_each(ranges::views::slice(args, 1, 4), [](std::string_view arg){ logger.debug("found arg: {}", arg); });
    try
    {
        const auto results = parse(options, args);
        logger.debug("a = {}", results.get<'a'>());
        logger.debug("b = {}", results.get<'b'>());
        logger.debug("c = {}", results.get<'c'>());
        logger.debug("d = {}", results.get<'d'>());
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
