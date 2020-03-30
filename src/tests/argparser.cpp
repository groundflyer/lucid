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
ExpectedValues
{
    std::string_view keyword;
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

    int idx = 0;
    int _argc = 0;
    char **_argv = nullptr;

public:
    ArgsRange() = default;

    ArgsRange(int argc, char* argv[]) noexcept : _argc(argc), _argv(argv) {}

    char**
    read() const noexcept
    {
        return _argv + idx;
    }

    bool
    equal(ranges::default_sentinel_t) const noexcept
    {
        return idx == _argc;
    }

    void
    next() noexcept
    {
        ++idx;
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
    char ** words = nullptr;
    std::size_t idx = 0ul;
    bool is_set = false;

    constexpr void
    increment() noexcept
    {
        is_set |= idx == (nvals - 1);
        idx = (idx + 1ul) % nvals;        
    }

    template <std::size_t ... Idxs>
    constexpr value_type
    convert(std::index_sequence<Idxs...>) const noexcept
    {
        return value_type{converter(words[Idxs])...};
    }

public:
    explicit constexpr
    Binding(const value_type& value) noexcept : default_value(value) {}

    constexpr void
    set(std::string_view) noexcept
    {
        increment();
    }

    constexpr void
    set(char **data) noexcept
    {
        if (!words)
            words = data;
        increment();
    }

    value_type
    operator()() const
    {
        return is_set ? convert(std::make_index_sequence<nvals>{}) : default_value;
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

    constexpr void
    set(std::string_view new_word) noexcept
    {
        is_set = true;
        word = new_word;
    }

    constexpr void
    set(char **new_word) noexcept
    {
        set(*new_word);
    }

    constexpr value_type
    operator()() const
    {
        if constexpr (std::is_same_v<value_type, bool>)
            // flip default bool value or convert word if it's available
            return word.empty() ? (default_value ^ is_set) : converter(word);
        else
            return is_set ? converter(word) : default_value;
    }
};


template <typename Converter>
class Binding<Converter, -1ul>
{
    class value_range : public ranges::view_facade<value_range>
    {
        friend ranges::range_access;
        using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

        Converter converter;
        char** data = nullptr;
        std::size_t size = 0ul;
        std::size_t idx = 0ul;

        value_type
        read() const noexcept
        {
            return converter(data[idx]);
        }

        bool
        equal(ranges::default_sentinel_t) const noexcept
        {
            return idx == size;
        }

        void
        next() noexcept
        {
            ++idx;
        }

    public:
        value_range() = default;

        value_range(char** _data, std::size_t _size) : data(_data), size(_size) {}
    };

    Converter converter;
    char** data;
    std::size_t size = 0ul;

    constexpr void
    increment() noexcept
    {
        ++size;
    }

public:
    constexpr void
    set(std::string_view) noexcept
    {
        increment();
    }

    constexpr void
    set(char** _data) noexcept
    {
        if(!data)
            data = _data;

        increment();
    }

    constexpr value_range
    operator()() const noexcept
    {
        return value_range(data, size);
    }
};


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

template <char Key, typename Converter>
struct Option<Key, Converter, -1ul>
{
    using converter = Converter;
    static constexpr char key = Key;

    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr
    Option(const std::string_view _keyword, const std::string_view _doc, const std::string_view _var) noexcept : keyword(_keyword), doc(_doc), var(_var) {}

    constexpr Binding<Converter, -1ul>
    binding() const noexcept
    {
        return Binding<Converter, -1ul>();
    }

    constexpr std::size_t
    num_vals() const noexcept
    {
        return -1ul;
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
    constexpr decltype(auto)
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


template <typename ... Bindings, typename Word>
constexpr auto
set_binding(std::tuple<Bindings...>& bindings, const std::size_t token, Word word)
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
    char **data;

    std::string_view
    value() const noexcept
    {
        return *data;
    }
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
arg_case(char **data) noexcept
{
    std::string_view word{*data};
    std::size_t pos = 0ul;

    while (word[pos] == '-' && pos <= 2ul)
        ++pos;

    ParsedWord ret = Value{data};

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
                ret = KeywordValue{word.substr(pos, eq_pos - 2ul), word.substr(eq_pos + 1ul)};
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
        if (token != -1ul && nvals != -1ul && nvals > 0ul)
            throw ExpectedValues{visit(token, [](const auto& option){ return option.keyword; }, options)};
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
        expectation_check();
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
        logger.debug("Found value {}", value.value());
        auto& [token, nvals] = current_key;
        // we send char** data here because we need
        // to have access to neigbour args
        // in multi arg options
        set_binding(bindings, token, value.data);
        if (nvals != -1ul)
            --nvals;
        if (nvals == 0ul)
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
        char **data = args.read();
        std::string_view word{*data};

        if (word == "--")
        {
            only_values = true;
            continue;
        }

        const ParsedWord parsed_word = only_values ? Value{data} : arg_case(data);

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
                        Option<'f', identity, 2>({"defval1", "defval2"}, "2val", "", ""),
                        Option<'m', identity, -1ul>("multi_val", "", "")};

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
        auto multi_range = results.get<'m'>();
        logger.debug("a = {}", results.get<'a'>());
        logger.debug("b = {}", results.get<'b'>());
        logger.debug("c = {}", results.get<'c'>());
        logger.debug("d = {}", results.get<'d'>());
        logger.debug("multival = {}, {}", f1, f2);
        logger.debug("multirange values:");
        ranges::for_each(multi_range, [&](auto&& val){ logger.debug("{}", val); });
    }
    catch (const KeyException<char>& ex)
    {
        logger.critical("Unknown key: {}", ex.value);
    }
    catch (const KeyException<std::string_view>& ex)
    {
        logger.critical("Unknown keyword: {}", ex.value);
    }
    catch (const ExpectedValues& ex)
    {
        logger.critical("Expected values for {}", ex.keyword);
    }
    return 0;
}
