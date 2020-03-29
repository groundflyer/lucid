// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/tuple.hpp>
#include <utils/logging.hpp>

#include <array>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <utility>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/facade.hpp>
#include <range/v3/view/slice.hpp>


namespace lucid::argparse
{

template <typename Key>
struct
KeyException
{
    Key value;
};

struct
Exception
{
    std::string_view what;
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

    throw KeyException<char>{key};
}

template <std::size_t Idx, typename ... Options>
constexpr std::size_t
tokenize_impl(const std::string_view keyword, const std::tuple<Options...>& options)
{
    if (keyword == std::get<Idx>(options).keyword)
        return Idx;

    if constexpr (Idx < sizeof...(Options) - 1)
        return tokenize_impl<Idx+1>(keyword, options);

    throw KeyException<std::string_view>{keyword};
}
} // detail


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


template <typename Converter, std::size_t nvals>
class Binding
{
    using value_type = std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>, nvals>;

    value_type default_value;
    Converter converter;
    std::array<std::string_view, nvals> words;
    std::size_t idx = 0ul;
    bool is_set = false;

public:
    explicit constexpr
    Binding(const value_type& value) noexcept : default_value(value) {}

    void
    set(std::string_view new_word) noexcept
    {
        is_set |= idx == (nvals - 1);
        words[idx] = new_word;
        idx = (idx + 1ul) % nvals;
    }

    value_type
    operator()() const
    {
        return is_set ? std::apply([&](const auto ... word){ return value_type{converter(word)...}; }, words) : default_value;
    }
};


template <typename Converter>
class Binding<Converter, 1>
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
            return word.empty() ? (default_value ^ is_set) : converter(word);
        else
            return is_set ? converter(word) : default_value;
    }
};


// template <typename Converter>
// class Binding<Converter, -1ul> : public ranges::view_facade<Binding<Converter, -1ul>>
// {
//     using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

//     friend ranges::range_access;

//     Converter converter;

//     value_type
//     read() const noexcept
//     {
//         return converter(words_range.read());
//     }

//     bool
//     equal(ranges::default_sentinel_t sentinel) const noexcept
//     {
//         return words_range.equal(sentinel);
//     }

//     void
//     next() noexcept
//     {
//         words_range.next();
//     }

// public:
//     Binding() = default;

//     ArgsRange words_range;
// };


template <char Key, typename Converter, std::size_t nvals = 1>
struct Option
{
    using converter = Converter;
    using singular_value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;
    using value_type = std::conditional_t<(nvals > 1), std::array<singular_value_type, nvals>, singular_value_type>;
    static constexpr char key = Key;

    value_type default_value;
    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr
    Option(const value_type& _default_value, const std::string_view _keyword, const std::string_view _doc, const std::string_view _var) noexcept :
        default_value(_default_value), keyword(_keyword), doc(_doc), var(_var)
    {}

    constexpr Binding<Converter, nvals>
    binding() const noexcept
    {
        return Binding<Converter, nvals>(default_value);
    }

    constexpr std::size_t
    num_vals() const noexcept
    {
        // no values required if is Flag
        return (std::is_same_v<bool, value_type> && (nvals == 1ul)) ? 0ul : nvals;
    }
};

template <char Key>
using Flag = Option<Key, FlagConverter>;


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


template <typename ... Options>
std::size_t
num_vals(const std::size_t token, const std::tuple<Options...> options)
{
    return visit(token, [](const auto& option){ return option.num_vals(); }, options);
}

struct
KeyInfo
{
    std::size_t token;
    std::size_t nvals;
};

template <typename Key, typename Options>
KeyInfo
key_info(const Key key, const Options& options)
{
    const std::size_t token = tokenize(key, options);
    const std::size_t nvals = num_vals(token, options);
    return KeyInfo{token, nvals};
}


template <typename ... Bindings>
auto
set_binding(std::tuple<Bindings...>& bindings, const std::size_t token, const std::string_view word)
{
    return visit(token, [&](auto& binding){ binding.set(word); }, bindings);
}


static inline Logger logger(Logger::DEBUG);

template <typename KeyType>
struct
Key
{
    KeyType value;
};

struct
KeywordValue
{
    std::string_view keyword;
    std::string_view value;
};

struct
Value
{
    std::string_view value;
};

struct
KeyCharSeq
{
    std::string_view seq;
};

using KeyChar = Key<char>;
using Keyword = Key<std::string_view>;
using ParsedWord = std::variant<KeyChar, KeyCharSeq, Keyword, KeywordValue, Value>;

ParsedWord
arg_case(std::string_view word) noexcept
{
    std::size_t pos = 0ul;

    while (word[pos] == '-' && pos <= 2ul)
        ++pos;

    ParsedWord ret = Value{word};

    switch (pos)
    {
        // -
    case 1ul:
        {
        if (word.size() > 2ul)
            // -abcd
            ret = KeyCharSeq{word.substr(pos)};
        else
            // -a
            ret = KeyChar{word[pos]};
        }
        break;
            // --
    case 2ul:
        {
            const std::size_t eq_pos = word.find('=');
            if (eq_pos == std::string_view::npos)
                // --keyword
                ret = Keyword{word.substr(pos)};
            else
                // --keyword=value
                ret = KeywordValue{word.substr(pos, eq_pos-2ul), word.substr(eq_pos + 1ul)};
        }
        break;
    }

    return ret;
}


template <typename ... Options>
auto
make_bindings(const std::tuple<Options...>& options) noexcept
{
    return std::apply([](const Options& ... options)
                      {
                          return std::tuple{options.binding()...};
                      }, options);
}


template <typename Options>
class
Visitor
{
    using Bindings = std::decay_t<decltype(make_bindings(std::declval<Options>()))>;
    Bindings bindings;
    const Options& options;
    KeyInfo current_key{-1ul, 0ul};

    void
    expectation_check() const
    {
        const auto& [token, nvals] = current_key;
        if (token != -1ul && nvals > 0ul)
            throw Exception{"Expected value, got key"};
    }

    void
    reset_key()
    {
        current_key = KeyInfo{-1ul, 0ul};
    }

public:
    constexpr
    Visitor(const Options& _options) :
        bindings(make_bindings(_options)), options(_options) {}

    Visitor() = delete;
    Visitor(const Visitor&) = delete;
    Visitor(Visitor&&) = delete;
    Visitor& operator=(const Visitor&) = delete;

    constexpr Bindings
    get_bindings() const
    {
        const auto& [token, nvals] = current_key;
        if (token != -1ul && nvals != 0ul)
            throw Exception{"Required arguments not set"};
            
        return bindings;
    }

    template <typename T>
    void
    operator()(const Key<T>& key)
    {
        expectation_check();

        logger.debug("Found key {}", key.value);

        const KeyInfo new_key = key_info(key.value, options);
        const auto& [new_token, new_nvals] = new_key;

        // is flag
        if (new_nvals == 0ul)
        {
            logger.debug("Flipping flag {}", key.value);
            set_binding(bindings, new_token, "");
            reset_key();
        }
        else
        {
            logger.debug("Saving token {}", new_token);
            current_key = new_key;
        }
    }

    void
    operator()(const KeywordValue& keyval)
    {
        logger.debug("Found {}={}", keyval.keyword, keyval.value);
        expectation_check();

        const std::size_t new_token = tokenize(keyval.keyword, options);
        set_binding(bindings, new_token, keyval.value);
        reset_key();
    }

    void
    operator()(const Value& value)
    {
        logger.debug("Found value {}", value.value);
        set_binding(bindings, current_key.token, value.value);
        --current_key.nvals;
        if (current_key.nvals == 0ul)
            reset_key();
    }

    void
    operator()(const KeyCharSeq& value)
    {
        expectation_check();
        logger.debug("found keychar sequence {}", value.seq);
        for (std::size_t i = 0ul; i < value.seq.size(); ++i)
        {
            const KeyInfo cki = key_info(value.seq[i], options);
            const auto [token, nvals] = cki;

            if (nvals == 1ul)
            {
                if (i == value.seq.size() - 1)
                {
                    // at the end
                    current_key = cki;
                }
                else
                {
                    set_binding(bindings, token, value.seq.substr(i + 1));
                    break;
                }
            }
            else
            {
                // flip flag
                set_binding(bindings, token, "");
            }
        }
    }
};


template <typename ... Options>
auto
parse(const std::tuple<Options...>& options, ArgsRange args)
{
    Visitor visitor{options};
    bool only_values = false;

    while(!args.equal(ranges::default_sentinel))
    {
        std::string_view word = args.read();

        logger.debug("Iterating word {}", word);

        if (word == "--")
        {
            only_values = true;
            continue;
        }

        const ParsedWord parsed_word = only_values ? Value{word} : arg_case(word);

        std::visit(visitor, parsed_word);

        args.next();
    }

    return ParseResults(visitor.get_bindings(), options);
}

} // namespace lucid::argparse

using namespace std;
using namespace lucid;
using namespace lucid::argparse;

constexpr tuple options{Option<'a', identity>("foo", "foo", "", ""),
                        Option<'b', identity>("bar", "bar", "", ""),
                        Option<'c', identity>("foo2", "foo2", "", ""),
                        Flag<'d'>(false, "flag", "", ""),
                        Option<'f', identity, 2>({"defval1", "defval2"}, "multival", "", "")};

static_assert(!has_repeating<decay_t<decltype(options)>>::value);

int main(int argc, char *argv[])
{
    ArgsRange args(argc, argv);
    args.next();
    // ranges::for_each(ranges::views::slice(args, 1, 4), [](std::string_view arg){ logger.debug("found arg: {}", arg); });
    try
    {
        const auto results = parse(options, args);
        const auto [f1, f2] = results.get<'f'>();
        logger.debug("a = {}", results.get<'a'>());
        logger.debug("b = {}", results.get<'b'>());
        logger.debug("c = {}", results.get<'c'>());
        logger.debug("d = {}", results.get<'d'>());
        logger.debug("multival = {}, {}", f1, f2);
    }
    catch (const KeyException<char>& ex)
    {
        logger.critical("Unknown key: {}", ex.value);
    }
    catch (const KeyException<std::string_view>& ex)
    {
        logger.critical("Unknown keyword: {}", ex.value);
    }
    catch (const Exception& ex)
    {
        logger.critical("Error: {}", ex.what);
    }
    return 0;
}
