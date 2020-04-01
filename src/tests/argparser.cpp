// -*- C++ -*-
// argparser.cpp
#include <utils/identity.hpp>
#include <utils/tuple.hpp>
#include <utils/logging.hpp>

#include <array>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <utility>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/facade.hpp>


namespace lucid::argparse
{

template <typename Key>
struct
KeyException
{
    Key value;
};

struct
FewArgumentsException
{
    std::string_view keyword;
};

struct
UnexpectedValue
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

template <typename Converter, std::size_t ... Idxs>
constexpr auto
argc2array(char* argc[], Converter converter, std::index_sequence<Idxs...>) noexcept
{
    using value_type = std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>, sizeof...(Idxs)>;
    return value_type{converter(argc[Idxs])...};
}
} // detail


class ArgsRange : public ranges::view_facade<ArgsRange>
{
    friend ranges::range_access;

    int idx = 0;
    int _argc = 0;
    char **_argv = nullptr;

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

public:
    ArgsRange() = default;

    ArgsRange(int argc, char* argv[]) noexcept : _argc(argc), _argv(argv) {}

    void
    next() noexcept
    {
        ++idx;
    }
};


template <typename Converter, std::size_t nvals>
class Binding
{
    using value_type = std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>, nvals>;

    Converter converter;
    value_type default_value;
    char ** words = nullptr;
    std::size_t idx = 0ul;
    bool is_set = false;

    constexpr void
    increment() noexcept
    {
        is_set |= idx == (nvals - 1);
        idx = (idx + 1ul) % nvals;        
    }

public:
    explicit constexpr
    Binding(const Converter& _converter, const value_type& value) noexcept : converter(_converter), default_value(value) {}

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

    constexpr value_type
    operator()() const
    {
        return is_set ? detail::argc2array(words, converter, std::make_index_sequence<nvals>{}) : default_value;
    }
};


template <typename Converter>
class Binding<Converter, 1>
{
    using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    Converter converter;
    value_type default_value;
    std::string_view word;
    bool is_set = false;

public:
    explicit constexpr
    Binding(const Converter& _converter, const value_type& value) noexcept : converter(_converter), default_value(value) {}

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

        Converter converter;
        char** data = nullptr;
        std::size_t size = 0ul;
        std::size_t idx = 0ul;

        decltype(auto)
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

        explicit
        value_range(const Converter& _converter, char*_data[], std::size_t _size) noexcept : converter(_converter), data(_data), size(_size) {}
    };

    Converter converter;
    char** data = nullptr;
    std::size_t size = 0ul;

    constexpr void
    increment() noexcept
    {
        ++size;
    }

public:
    constexpr explicit
    Binding(const Converter& _converter) noexcept : converter(_converter) {}

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
        return value_range(converter, data, size);
    }
};


template <typename Converter, std::size_t nvals>
class PositionalBinding
{
    Converter converter;
    char ** words = nullptr;
    std::size_t idx = 0ul;

public:
    explicit constexpr
    PositionalBinding(const Converter& _converter) noexcept : converter(_converter) {}

    constexpr void
    set(std::string_view) noexcept
    {
        ++idx;
    }

    constexpr void
    set(char **data) noexcept
    {
        if (!words)
            words = data;
        ++idx;
    }

    constexpr auto
    operator()() const
    {
        return detail::argc2array(words, converter, std::make_index_sequence<nvals>{});
    }
};

template <typename Converter>
class PositionalBinding<Converter, 1>
{
    Converter converter;
    std::string_view word;

public:
    explicit constexpr
    PositionalBinding(const Converter& _converter) noexcept : converter(_converter) {};

    constexpr void
    set(std::string_view new_word) noexcept
    {
        word = new_word;
    }

    constexpr void
    set(char** data) noexcept
    {
        set(*data);
    }

    constexpr auto
    operator()() const
    {
        return converter(word);
    }
};


template <char Key, typename Converter, std::size_t nvals>
struct Option
{
    using singular_value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;
    using value_type = std::conditional_t<(nvals > 1), std::array<singular_value_type, nvals>, singular_value_type>;
    static constexpr char key = Key;

    Converter converter;
    value_type default_value;
    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr
    Option(Converter&& _converter,
           const value_type& _default_value,
           const std::string_view _keyword,
           const std::string_view _doc,
           const std::string_view _var) noexcept :
        converter(_converter), default_value(_default_value), keyword(_keyword), doc(_doc), var(_var)
    {}

    constexpr Binding<Converter, nvals>
    binding() const noexcept
    {
        return Binding<Converter, nvals>(converter, default_value);
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
    static constexpr char key = Key;

    Converter converter;
    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr
    Option(Converter&& _converter,
           const std::string_view _keyword,
           const std::string_view _doc,
           const std::string_view _var) noexcept :
        converter(_converter), keyword(_keyword), doc(_doc), var(_var) {}

    constexpr Binding<Converter, -1ul>
    binding() const noexcept
    {
        return Binding<Converter, -1ul>(converter);
    }

    constexpr std::size_t
    num_vals() const noexcept
    {
        return -1ul;
    }
};

template <typename Converter, std::size_t nvals>
struct
Positional
{
    Converter converter;
    std::string_view doc;
    std::string_view var;

    constexpr
    Positional(Converter&& _converter,
               const std::string_view _doc,
               const std::string_view _var) noexcept :
        converter(_converter), doc(_doc), var(_var) {}

    constexpr auto
    binding() const noexcept
    {
        if constexpr (nvals == -1ul)
            return Binding<Converter, -1ul>(converter);
        else
            return PositionalBinding<Converter, nvals>(converter);
    }

    constexpr std::size_t
    num_vals() const noexcept
    {
        return nvals;
    }
};


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

template <typename ... Options>
struct has_repeating
{
    static constexpr bool value = detail::has_repeating_impl<Options...>::value;
};


template <typename OptBindings, typename PosBindings, typename Options>
struct ParseResults
{
    OptBindings opt_bindings;
    PosBindings pos_bindings;

    constexpr
    ParseResults(const std::pair<OptBindings, PosBindings>& _bindings, const Options&) : opt_bindings(_bindings.first), pos_bindings(_bindings.second) {}

    template <char Key>
    constexpr decltype(auto)
    get_opt() const noexcept
    {
        return std::get<key_index<Key, Options>::value>(opt_bindings)();
    }

    template <std::size_t Idx>
    constexpr decltype(auto)
    get_pos() const noexcept
    {
        return std::get<Idx>(pos_bindings)();
    }
};


template <typename ... Bindings, typename Word>
constexpr auto
set_binding(std::tuple<Bindings...>& bindings, const std::size_t token, Word word) noexcept
{
    return visit_clamped(token, [&](auto& binding){ binding.set(word); }, bindings);
}


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
    string() const noexcept
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


template <typename OptDesc, typename PosDesc>
class
Visitor
{
    using OptBindings = std::decay_t<decltype(make_bindings(std::declval<OptDesc>()))>;
    using PosBindings = std::decay_t<decltype(make_bindings(std::declval<PosDesc>()))>;
    static constexpr std::size_t num_pos = std::tuple_size_v<PosDesc>;
    static constexpr bool has_pos = num_pos > 0ul;

    template <typename Desc, typename Bindings>
    struct
    State
    {
        const Desc& desc;
        Bindings bindings;
        std::size_t token;
        std::size_t remain;

        constexpr
        State(const Desc& _desc, std::size_t _token, std::size_t _remain) :
            desc(_desc), bindings(make_bindings(_desc)), token(_token), remain(_remain) {}

        State() = delete;
        State(const State&) = delete;
        State(State&&) = delete;
        State& operator=(const State&) = delete;
    };

    State<OptDesc, OptBindings> opt_state;
    State<PosDesc, PosBindings> pos_state;
    bool is_pos = has_pos;

    template <typename Desc>
    static std::size_t
    get_nvals(const std::size_t _token, const Desc& desc)
    {
        return visit_clamped(_token, [](const auto& desc){ return desc.num_vals(); }, desc);
    }

    template <typename Desc>
    std::string_view
    get_keyword(std::size_t _token, const Desc& opts_desc) const noexcept
    {
        return visit_clamped(_token, [](const auto& option){ return option.keyword; }, opts_desc);
    }

    void
    check_opt() const
    {
        if (opt_state.token != -1ul && opt_state.remain != -1ul && opt_state.remain > 0ul)
            throw FewArgumentsException{get_keyword(opt_state.token, opt_state.desc)};
    }

    bool
    pos_not_set() const noexcept
    {
        return has_pos && pos_state.token < num_pos;
    }

    void
    check_pos() const
    {
        if (pos_not_set())
            throw FewArgumentsException{"Positional arguments not set"};
    }

    void
    reset_opt() noexcept
    {
        opt_state.token = -1ul;
        opt_state.remain = 0ul;
        is_pos = pos_not_set();
    }

    template <typename Key>
    constexpr auto
    key_info(const Key key)
    {
        const std::size_t new_token = tokenize(key, opt_state.desc);
        const std::size_t new_nvals = get_nvals(new_token, opt_state.desc);
        return std::pair{new_token, new_nvals};
    }

public:
    constexpr
    Visitor(const OptDesc& opt_desc, const PosDesc& pos_desc) :
        opt_state(opt_desc, -1ul, 0ul),
        pos_state(pos_desc, 0ul, has_pos ? get_nvals(0ul, pos_desc) : 0ul) {}

    Visitor() = delete;
    Visitor(const Visitor&) = delete;
    Visitor(Visitor&&) = delete;
    Visitor& operator=(const Visitor&) = delete;

    constexpr std::pair<OptBindings, PosBindings>
    get_bindings() const
    {
        check_opt();
        check_pos();
        return std::pair{opt_state.bindings, pos_state.bindings};
    }

    template <typename T>
    void
    operator()(const Key<T>& key)
    {
        check_opt();

        const auto [new_token, new_nvals] = key_info(key.value);

        // is flag
        if (new_nvals == 0ul)
        {
            set_binding(opt_state.bindings, new_token, "");
            reset_opt();
        }
        else
        {
            opt_state.token = new_token;
            opt_state.remain = new_nvals;
            is_pos = false;
        }
    }

    void
    operator()(const KeywordValue& keyval)
    {
        check_opt();

        const auto [new_token, new_nvals] = key_info(keyval.keyword);

        if (new_nvals > 1ul)
            throw FewArgumentsException{keyval.keyword};

        set_binding(opt_state.bindings, new_token, keyval.value);
        reset_opt();
    }

    void
    operator()(const Value& value)
    {
        auto state_update = [&](auto& state, const auto& token_check, const auto& reset)
                            {
                                if (token_check(state.token))
                                    throw UnexpectedValue{value.string()};

                                // we send char** data here because we need
                                // to have access to neigbour args
                                // in multi arg options
                                set_binding(state.bindings, state.token, value.data);

                                state.remain -= (state.remain != -1ul);

                                if (state.remain == 0ul)
                                    reset(*this);
                            };

        if (is_pos)
            state_update(pos_state,
                         [&](std::size_t token){ return token >= num_pos; },
                         [&](Visitor&){
                             ++pos_state.token;
                             pos_state.remain = get_nvals(pos_state.token, pos_state.desc);
                             is_pos = pos_not_set();
                         });
        else
            state_update(opt_state, [&](std::size_t token){ return token == -1ul; }, std::mem_fn(&Visitor::reset_opt));
    }

    void
    operator()(const KeyCharSeq& value)
    {
        check_opt();
        for (std::size_t i = 0ul; i < value.seq.size(); ++i)
        {
            const auto [new_token, new_nvals] = key_info(value.seq[i]);

            if (new_nvals > 1ul)
                throw FewArgumentsException{get_keyword(new_token, opt_state.desc)};

            if (opt_state.remain == 1ul)
            {
                if (i == value.seq.size() - 1)
                {
                    // at the end
                    opt_state.token = new_token;
                    opt_state.remain = new_nvals;
                }
                else
                {
                    set_binding(opt_state.bindings, new_token, value.seq.substr(i + 1ul));
                    break;
                }
            }
            else
            {
                // flip flag
                set_binding(opt_state.bindings, new_token, "");
            }
        }
    }
};


template <typename ... Options, typename ... Positionals>
auto
parse(const std::tuple<Options...>& options, const std::tuple<Positionals...>& positionals, ArgsRange args)
{
    static_assert(!has_repeating<Options...>::value, "All keys should be uinique");

    Visitor visitor{options, positionals};
    bool only_values = false;

    auto update = [&](char** data)
    {
        std::string_view word{*data};

        if (word == "--")
            only_values = true;
        else
            std::visit(visitor, only_values ? Value{data} : arg_case(data));
    };

    ranges::for_each(args, update);

    return ParseResults(visitor.get_bindings(), options);
}

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

template <char Key, typename Converter, typename ... Args>
constexpr Option<Key, Converter, 1ul>
option(Converter&& converter, Args&& ... args) noexcept
{
    return Option<Key, Converter, 1ul>(std::forward<Converter>(converter), std::forward<Args>(args)...);
}

template <char Key, std::size_t N, typename Converter, typename ... Args>
constexpr Option<Key, Converter, N>
option(Converter&& converter, const std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>, N>& def_vals, Args&& ... args) noexcept
{
    static_assert(N > 1ul && N != -1ul, "Use option without N argument");
    return Option<Key, Converter, N>(std::forward<Converter>(converter), def_vals, std::forward<Args>(args)...);
}

template <char Key, typename Converter, typename ... Args>
constexpr Option<Key, Converter, -1ul>
option_list(Converter&& converter, Args&& ... args) noexcept
{
    return Option<Key, Converter, -1ul>(std::forward<Converter>(converter), std::forward<Args>(args)...);
}

template <char Key, typename ... Args>
constexpr Option<Key, FlagConverter, 1ul>
flag(Args&& ... args) noexcept
{
    return Option<Key, FlagConverter, 1ul>(FlagConverter{}, std::forward<Args>(args)...);
}

template <typename Converter, typename ... Args>
constexpr Positional<Converter, 1ul>
positional(Converter&& converter, Args&& ... args) noexcept
{
    return Positional<Converter, 1ul>(std::forward<Converter>(converter), std::forward<Args>(args)...);
}

template <std::size_t nvals, typename Converter, typename ... Args>
constexpr Positional<Converter, nvals>
positional(Converter&& converter, Args&& ... args) noexcept
{
    return Positional<Converter, nvals>(std::forward<Converter>(converter), std::forward<Args>(args)...);
}

template <typename Converter, typename ... Args>
constexpr Positional<Converter, -1ul>
positional_list(Converter&& converter, Args&& ... args) noexcept
{
    return Positional<Converter, -1ul>(std::forward<Converter>(converter), std::forward<Args>(args)...);
}
} // namespace lucid::argparse

using namespace std;
using namespace lucid;
using namespace lucid::argparse;

constexpr tuple options{option<'a'>(identity{}, "default value for foo", "foo", "", ""),
                        option<'b'>(identity{}, "default value for bar", "bar", "", ""),
                        option<'c'>(identity{}, "default value for foo2", "foo2", "", ""),
                        flag<'d'>(false, "flag", "", ""),
                        option<'f', 2ul>(identity{}, {"defval1", "defval2"}, "2val", "", ""),
                        option_list<'m'>(identity{}, "multi_val", "", "")};

constexpr tuple positionals{positional<2ul>(identity{}, "doc for 1st positional", "pos1"),
                            positional(identity{}, "doc for 2nd positional", "pos2")};

static inline Logger logger(Logger::DEBUG);

int main(int argc, char *argv[])
{
    ArgsRange args(argc, argv);
    args.next();
    try
    {
        const auto results = parse(options, positionals, args);
        const auto [f1, f2] = results.get_opt<'f'>();
        auto multi_range = results.get_opt<'m'>();
        const auto [p1, p2] = results.get_pos<0>();
        const auto p3 = results.get_pos<1>();
        logger.debug("a = {}", results.get_opt<'a'>());
        logger.debug("b = {}", results.get_opt<'b'>());
        logger.debug("c = {}", results.get_opt<'c'>());
        logger.debug("d = {}", results.get_opt<'d'>());
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
    catch (const FewArgumentsException& ex)
    {
        logger.critical("Expected values for {}", ex.keyword);
    }
    return 0;
}
