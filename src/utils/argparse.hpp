// -*- C++ -*-
// argparse.hpp
//

#pragma once

#include <utils/logging.hpp>
#include <utils/tuple.hpp>

#include <array>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/facade.hpp>

#include <fmt/printf.h>

namespace lucid::argparse
{

template <typename Key>
struct KeyException
{
    Key value;
};

struct FewArgumentsException
{
    std::string_view keyword;
};

struct UnexpectedValue
{
    std::string_view value;
};

struct NeedHelpException
{
};

namespace detail
{
template <char Key, std::size_t Idx, typename... Options>
struct key_index_impl;

template <char Key, std::size_t Idx>
struct key_index_impl<Key, Idx>
{
    static constexpr std::size_t value = -1ul;
};

template <char Key, std::size_t Idx, typename _Option, typename... Rest>
struct key_index_impl<Key, Idx, _Option, Rest...>
{
    static constexpr std::size_t value =
        (Key == _Option::key) ? Idx : key_index_impl<Key, Idx + 1, Rest...>::value;
};

template <typename... Options>
struct has_repeating_key_impl;

template <typename First, typename... Rest>
struct has_repeating_key_impl<First, Rest...>
{
    static constexpr bool value =
        (false || ... || (First::key == Rest::key)) || has_repeating_key_impl<Rest...>::value;
};

template <typename Last>
struct has_repeating_key_impl<Last>
{
    static constexpr bool value = false;
};

template <std::size_t Idx, typename FirstOption, typename... RestOptions>
constexpr std::size_t
tokenize_impl(const char key)
{
    if(key == FirstOption::key) return Idx;

    if constexpr(sizeof...(RestOptions) > 0ul) return tokenize_impl<Idx + 1, RestOptions...>(key);

    throw KeyException<char>{key};
}

template <std::size_t Idx, typename... Options>
constexpr std::size_t
tokenize_impl(const std::string_view keyword, const std::tuple<Options...>& options)
{
    if(keyword == std::get<Idx>(options).keyword) return Idx;

    if constexpr(Idx < sizeof...(Options) - 1) return tokenize_impl<Idx + 1>(keyword, options);

    throw KeyException<std::string_view>{keyword};
}

template <typename Converter, std::size_t... Idxs>
constexpr auto
argv2array(char* argc[], Converter converter, std::index_sequence<Idxs...>) noexcept
{
    using value_type = std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>,
                                  sizeof...(Idxs)>;
    return value_type{converter(argc[Idxs])...};
}

template <typename T>
struct fmt_options
{
    const T& value;
};

template <typename T>
struct fmt_pos
{
    const T& value;
};

template <typename Options, typename Positionals>
struct fmt_data
{
    std::string_view     program_name;
    fmt_options<Options> options;
    fmt_pos<Positionals> positionals;

    fmt_data()                = delete;
    fmt_data(const fmt_data&) = delete;
    fmt_data(fmt_data&&)      = delete;
    fmt_data&
    operator=(const fmt_data&) = delete;

    constexpr fmt_data(std::string_view   _program_name,
                       const Options&     _options,
                       const Positionals& _positionals) noexcept :
        program_name(_program_name),
        options{_options}, positionals{_positionals}
    {
    }
};
} // namespace detail

class ArgsRange : public ranges::view_facade<ArgsRange>
{
    friend ranges::range_access;

    int    idx   = 0;
    int    _argc = 0;
    char** _argv = nullptr;

    bool equal(ranges::default_sentinel_t) const noexcept { return idx == _argc; }

  public:
    ArgsRange() = default;

    ArgsRange(int argc, char* argv[]) noexcept : _argc(argc), _argv(argv) {}

    char**
    read() const noexcept
    {
        return _argv + idx;
    }

    void
    next() noexcept
    {
        ++idx;
    }
};

template <typename Converter, std::size_t nvals>
class Binding
{
    using value_type =
        std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>, nvals>;

    Converter   converter;
    value_type  default_value;
    char**      words  = nullptr;
    std::size_t idx    = 0ul;
    bool        is_set = false;

    constexpr void
    increment() noexcept
    {
        is_set |= idx == (nvals - 1);
        idx = (idx + 1ul) % nvals;
    }

  public:
    explicit constexpr Binding(const Converter& _converter, const value_type& value) noexcept :
        converter(_converter), default_value(value)
    {
    }

    constexpr void set(std::string_view) noexcept { increment(); }

    constexpr void
    set(char** data) noexcept
    {
        if(!words) words = data;
        increment();
    }

    constexpr value_type
    operator()() const
    {
        return is_set ? detail::argv2array(words, converter, std::make_index_sequence<nvals>{}) :
                        default_value;
    }
};

template <typename Converter>
class Binding<Converter, 1>
{
    using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    Converter        converter;
    value_type       default_value;
    std::string_view word;
    bool             is_set = false;

  public:
    explicit constexpr Binding(const Converter& _converter, const value_type& value) noexcept :
        converter(_converter), default_value(value)
    {
    }

    constexpr void
    set(std::string_view new_word) noexcept
    {
        is_set = true;
        word   = new_word;
    }

    constexpr void
    set(char** new_word) noexcept
    {
        set(*new_word);
    }

    constexpr value_type
    operator()() const
    {
        if constexpr(std::is_same_v<value_type, bool>)
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

        Converter   converter;
        char**      data = nullptr;
        std::size_t size = 0ul;
        std::size_t idx  = 0ul;

        decltype(auto)
        read() const
        {
            return converter(data[idx]);
        }

        bool equal(ranges::default_sentinel_t) const noexcept { return idx == size; }

        void
        next() noexcept
        {
            ++idx;
        }

      public:
        value_range() = default;

        explicit value_range(const Converter& _converter,
                             char*            _data[],
                             std::size_t      _size) noexcept :
            converter(_converter),
            data(_data), size(_size)
        {
        }
    };

    Converter   converter;
    char**      data = nullptr;
    std::size_t size = 0ul;

    constexpr void
    increment() noexcept
    {
        ++size;
    }

  public:
    constexpr explicit Binding(const Converter& _converter) noexcept : converter(_converter) {}

    constexpr void set(std::string_view) noexcept { increment(); }

    constexpr void
    set(char** _data) noexcept
    {
        if(!data) data = _data;

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
    Converter   converter;
    char**      words = nullptr;
    std::size_t idx   = 0ul;

  public:
    explicit constexpr PositionalBinding(const Converter& _converter) noexcept :
        converter(_converter)
    {
    }

    constexpr void set(std::string_view) noexcept { ++idx; }

    constexpr void
    set(char** data) noexcept
    {
        if(!words) words = data;
        ++idx;
    }

    constexpr auto
    operator()() const
    {
        return detail::argv2array(words, converter, std::make_index_sequence<nvals>{});
    }
};

template <typename Converter>
class PositionalBinding<Converter, 1>
{
    Converter        converter;
    std::string_view word;

  public:
    explicit constexpr PositionalBinding(const Converter& _converter) noexcept :
        converter(_converter){};

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
    using value_type          = std::
        conditional_t<(nvals > 1), std::array<singular_value_type, nvals>, singular_value_type>;
    using var_type =
        std::conditional_t<(nvals > 1), std::array<std::string_view, nvals>, std::string_view>;
    static constexpr bool        is_flag  = std::is_same_v<bool, value_type> && (nvals == 1ul);
    static constexpr char        key      = Key;
    static constexpr std::size_t num_vals = is_flag ? 0ul : nvals;

    Converter        converter;
    value_type       default_value;
    std::string_view keyword;
    std::string_view doc;
    var_type         var;

    constexpr Option(Converter&&            _converter,
                     const value_type&      _default_value,
                     const std::string_view _keyword,
                     const std::string_view _doc,
                     const var_type&        _var) noexcept :
        converter(_converter),
        default_value(_default_value), keyword(_keyword), doc(_doc), var(_var)
    {
    }

    constexpr Binding<Converter, nvals>
    binding() const noexcept
    {
        return Binding<Converter, nvals>(converter, default_value);
    }
};

template <char Key, typename Converter>
struct Option<Key, Converter, -1ul>
{
    static constexpr std::size_t num_vals = -1ul;
    static constexpr char        key      = Key;
    static constexpr bool        is_flag  = false;
    using value_type = std::decay_t<std::invoke_result_t<Converter, std::string_view>>;

    Converter        converter;
    std::string_view keyword;
    std::string_view doc;
    std::string_view var;

    constexpr Option(Converter&&            _converter,
                     const std::string_view _keyword,
                     const std::string_view _doc,
                     const std::string_view _var) noexcept :
        converter(_converter),
        keyword(_keyword), doc(_doc), var(_var.empty() ? _keyword : _var)
    {
    }

    constexpr Binding<Converter, -1ul>
    binding() const noexcept
    {
        return Binding<Converter, -1ul>(converter);
    }
};

template <typename Converter, std::size_t nvals>
struct Positional
{
    using var_type                        = std::conditional_t<(nvals > 1 && nvals != -1ul),
                                        std::array<std::string_view, nvals>,
                                        std::string_view>;
    static constexpr std::size_t num_vals = nvals;

    Converter        converter;
    std::string_view doc;
    var_type         var;

    constexpr Positional(Converter&&            _converter,
                         const std::string_view _doc,
                         const var_type&        _var) noexcept :
        converter(_converter),
        doc(_doc), var(_var)
    {
    }

    constexpr auto
    binding() const noexcept
    {
        if constexpr(nvals == -1ul)
            return Binding<Converter, -1ul>(converter);
        else
            return PositionalBinding<Converter, nvals>(converter);
    }
};

template <char Key, typename Converter>
constexpr Option<Key, Converter, 1ul>
option(Converter&&                                             converter,
       const typename Option<Key, Converter, 1ul>::value_type& def_val,
       const std::string_view                                  keyword,
       const std::string_view                                  doc,
       const typename Option<Key, Converter, 1ul>::var_type&   var) noexcept
{
    return Option<Key, Converter, 1ul>(
        std::forward<Converter>(converter), def_val, keyword, doc, var);
}

template <char Key, std::size_t N, typename Converter>
constexpr Option<Key, Converter, N>
option(Converter&&                                           converter,
       const typename Option<Key, Converter, N>::value_type& def_val,
       const std::string_view                                keyword,
       const std::string_view                                doc,
       const typename Option<Key, Converter, N>::var_type&   var) noexcept
{
    static_assert(N > 1ul && N != -1ul, "Use option without N argument");
    return Option<Key, Converter, N>(
        std::forward<Converter>(converter), def_val, keyword, doc, var);
}

template <char Key, typename Converter, typename... Args>
constexpr Option<Key, Converter, -1ul>
option_list(Converter&&            converter,
            const std::string_view keyword,
            const std::string_view doc,
            const std::string_view var) noexcept
{
    return Option<Key, Converter, -1ul>(std::forward<Converter>(converter), keyword, doc, var);
}

template <char Key, typename... Args>
constexpr auto
flag(const bool def_val, const std::string_view keyword, const std::string_view doc) noexcept
{
    struct FlagConverter
    {
        struct Exception
        {
            std::string_view word;
        };

        constexpr bool
        operator()(std::string_view word) const
        {
            if(word == "yes") return true;

            if(word == "no") return false;

            throw Exception{word};
        }
    };

    return Option<Key, FlagConverter, 1ul>(FlagConverter{}, def_val, keyword, doc, "");
}

template <typename Converter>
constexpr Positional<Converter, 1ul>
positional(Converter&& converter, const std::string_view doc, const std::string_view var) noexcept
{
    return Positional<Converter, 1ul>(std::forward<Converter>(converter), doc, var);
}

template <std::size_t nvals, typename Converter>
constexpr Positional<Converter, nvals>
positional(Converter&&                                            converter,
           const std::string_view                                 doc,
           const typename Positional<Converter, nvals>::var_type& var) noexcept
{
    return Positional<Converter, nvals>(std::forward<Converter>(converter), doc, var);
}

template <typename Converter>
constexpr Positional<Converter, -1ul>
positional_list(Converter&&            converter,
                const std::string_view doc,
                const std::string_view var) noexcept
{
    return Positional<Converter, -1ul>(std::forward<Converter>(converter), doc, var);
}

template <char Key, typename Options>
struct key_index;

template <char Key, typename... Options>
struct key_index<Key, std::tuple<Options...>>
{
    static const constexpr std::size_t value = detail::key_index_impl<Key, 0ul, Options...>::value;
};

template <typename... Options>
constexpr std::size_t
tokenize(const char key, const std::tuple<Options...>)
{
    return detail::tokenize_impl<0ul, Options...>(key);
}

template <typename... Options>
constexpr std::size_t
tokenize(const std::string_view keyword, const std::tuple<Options...>& options)
{
    return detail::tokenize_impl<0ul>(keyword, options);
}

template <typename... Options>
struct has_repeating_key
{
    static constexpr bool value = detail::has_repeating_key_impl<Options...>::value;
};

template <typename... Options>
constexpr bool
keywords_have_space(const std::tuple<Options...>& options) noexcept
{
    return std::apply(
        [](const Options&... option) {
            return (false || ... || (option.keyword.find(' ') != std::string_view::npos));
        },
        options);
}

template <typename Bindings, typename Options>
struct ParseResults;

template <typename OptBindings, typename PosBindings, typename Options>
struct ParseResults<std::pair<OptBindings, PosBindings>, Options>
{
    OptBindings opt_bindings;
    PosBindings pos_bindings;

    constexpr ParseResults(const std::pair<OptBindings, PosBindings>& _bindings,
                           const Options&) noexcept :
        opt_bindings(_bindings.first),
        pos_bindings(_bindings.second)
    {
    }

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

template <typename... OptBindings, typename Options>
struct ParseResults<std::tuple<OptBindings...>, Options>
{
    std::tuple<OptBindings...> opt_bindings;

    constexpr ParseResults(const std::tuple<OptBindings...>& _bindings, const Options&) noexcept :
        opt_bindings(_bindings)
    {
    }

    template <char Key>
    constexpr decltype(auto)
    get_opt() const noexcept
    {
        return std::get<key_index<Key, Options>::value>(opt_bindings)();
    }
};

template <typename OptBindings, typename PosBindings, typename Options>
ParseResults(const std::pair<OptBindings, PosBindings>&, const Options&)
    ->ParseResults<std::pair<OptBindings, PosBindings>, Options>;

template <typename... OptBindings, typename Options>
ParseResults(const std::tuple<OptBindings...>&, const Options&)
    ->ParseResults<std::tuple<OptBindings...>, Options>;

template <typename... Bindings, typename Word>
constexpr auto
set_binding(std::tuple<Bindings...>& bindings, const std::size_t token, Word word) noexcept
{
    return visit_clamped(
        token, [&](auto& binding) { binding.set(word); }, bindings);
}

template <typename KeyType>
struct Key
{
    KeyType value;
};

struct KeywordValue
{
    std::string_view keyword;
    std::string_view value;
};

struct Value
{
    char** data;

    std::string_view
    string() const noexcept
    {
        return *data;
    }
};

struct KeyCharSeq
{
    std::string_view seq;
};

using KeyChar    = Key<char>;
using Keyword    = Key<std::string_view>;
using ParsedWord = std::variant<KeyChar, KeyCharSeq, Keyword, KeywordValue, Value>;

ParsedWord
arg_case(char** data) noexcept
{
    std::string_view word{*data};
    std::size_t      pos = 0ul;

    while(word[pos] == '-' && pos <= 2ul) ++pos;

    ParsedWord ret = Value{data};

    switch(pos)
    {
        // -
    case 1ul:
    {
        if(word.size() > 2ul)
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
        if(eq_pos == std::string_view::npos)
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

template <typename... Options>
auto
make_bindings(const std::tuple<Options...>& options) noexcept
{
    return std::apply([](const Options&... options) { return std::tuple{options.binding()...}; },
                      options);
}

template <typename OptDesc, typename PosDesc>
class Visitor
{
    static constexpr std::size_t num_pos = std::tuple_size_v<PosDesc>;
    static constexpr bool        has_pos = num_pos > 0ul;
    using OptBindings = std::decay_t<decltype(make_bindings(std::declval<OptDesc>()))>;
    using PosBindings =
        std::conditional_t<has_pos,
                           std::decay_t<decltype(make_bindings(std::declval<PosDesc>()))>,
                           void>;

    template <typename Desc, typename Bindings>
    struct State
    {
        const Desc& desc;
        Bindings    bindings;
        std::size_t token;
        std::size_t remain;

        constexpr State(const Desc& _desc, std::size_t _token, std::size_t _remain) noexcept :
            desc(_desc), bindings(make_bindings(_desc)), token(_token), remain(_remain)
        {
        }

        State()             = delete;
        State(const State&) = delete;
        State(State&&)      = delete;
        State&
        operator=(const State&) = delete;
    };

    // empty state if we have no positional arguments
    template <>
    struct State<std::tuple<>, void>
    {
        template <typename... Args>
        constexpr State(Args&&...) noexcept
        {
        }
    };

    State<OptDesc, OptBindings> opt_state;
    State<PosDesc, PosBindings> pos_state;
    bool                        is_pos = has_pos;

    template <typename... Ts>
    static std::size_t
    get_nvals(const std::size_t _token, const std::tuple<Ts...>& desc)
    {
        if constexpr(sizeof...(Ts) > 0)
            return visit_clamped(
                _token,
                [](const auto& desc) noexcept { return std::decay_t<decltype(desc)>::num_vals; },
                desc);
        else
            return 0ul;
    }

    template <typename Desc>
    std::string_view
    get_keyword(std::size_t _token, const Desc& opts_desc) const noexcept
    {
        return visit_clamped(
            _token, [](const auto& option) { return option.keyword; }, opts_desc);
    }

    bool
    pos_not_set() const noexcept
    {
        if constexpr(has_pos)
            return pos_state.token < num_pos;
        else
            return false;
    }

    void
    reset_opt() noexcept
    {
        opt_state.token  = -1ul;
        opt_state.remain = 0ul;
        is_pos           = pos_not_set();
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
    constexpr Visitor(const OptDesc& opt_desc, const PosDesc& pos_desc) :
        opt_state(opt_desc, -1ul, 0ul), pos_state(pos_desc, 0ul, get_nvals(0ul, pos_desc))
    {
    }

    Visitor()               = delete;
    Visitor(const Visitor&) = delete;
    Visitor(Visitor&&)      = delete;
    Visitor&
    operator=(const Visitor&) = delete;

    std::string_view
    check_opt() const noexcept
    {
        if(opt_state.token != -1ul && opt_state.remain != -1ul && opt_state.remain > 0ul)
            return get_keyword(opt_state.token, opt_state.desc);

        return "";
    }

    bool
    check_pos() const noexcept
    {
        return pos_not_set();
    }

    constexpr auto
    get_bindings() const
    {
        if constexpr(has_pos)
            return std::pair{opt_state.bindings, pos_state.bindings};
        else
            return opt_state.bindings;
    }

    template <typename T>
    void
    operator()(const Key<T>& key)
    {
        check_opt();

        const auto [new_token, new_nvals] = key_info(key.value);

        // is flag
        if(new_nvals == 0ul)
        {
            set_binding(opt_state.bindings, new_token, "");
            reset_opt();
        }
        else
        {
            opt_state.token  = new_token;
            opt_state.remain = new_nvals;
            is_pos           = false;
        }
    }

    void
    operator()(const KeywordValue& keyval)
    {
        check_opt();

        const auto [new_token, new_nvals] = key_info(keyval.keyword);

        if(new_nvals > 1ul) throw FewArgumentsException{keyval.keyword};

        set_binding(opt_state.bindings, new_token, keyval.value);
        reset_opt();
    }

    void
    operator()(const Value& value)
    {
        auto state_update = [&](auto& state, const auto& token_check, const auto& reset) {
            if(token_check(state.token)) throw UnexpectedValue{value.string()};

            // we send char** data here because we need
            // to have access to neigbour args
            // in multi arg options
            set_binding(state.bindings, state.token, value.data);

            state.remain -= (state.remain != -1ul);

            if(state.remain == 0ul) reset(*this);
        };

        if(is_pos)
        {
            if constexpr(has_pos)
                state_update(
                    pos_state,
                    [&](std::size_t token) { return token >= num_pos; },
                    [&](Visitor&) {
                        ++pos_state.token;
                        pos_state.remain = get_nvals(pos_state.token, pos_state.desc);
                        is_pos           = pos_not_set();
                    });
        }
        else
            state_update(
                opt_state,
                [&](std::size_t token) { return token == -1ul; },
                std::mem_fn(&Visitor::reset_opt));
    }

    void
    operator()(const KeyCharSeq& value)
    {
        check_opt();
        for(std::size_t i = 0ul; i < value.seq.size(); ++i)
        {
            const auto [new_token, new_nvals] = key_info(value.seq[i]);

            if(new_nvals > 1ul) throw FewArgumentsException{get_keyword(new_token, opt_state.desc)};

            if(opt_state.remain == 1ul)
            {
                if(i == value.seq.size() - 1)
                {
                    // at the end
                    opt_state.token  = new_token;
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

template <typename... Options, typename... Positionals, typename ErrorHandler>
auto
parse(const std::tuple<Options...>&     options,
      const std::tuple<Positionals...>& positionals,
      ArgsRange                         args,
      ErrorHandler&&                    error_handler) noexcept
{
    static_assert(!has_repeating_key<Options...>::value, "Keys must be unique");

    args.next();

    Visitor visitor{options, positionals};

    auto update = [&visitor, only_values = false](char** data) mutable {
        std::string_view word{*data};

        // very dirty way to stop iteration
        if(word == "-h" || word == "--help") throw NeedHelpException{};

        if(word == "--")
            only_values = true;
        else
            std::visit(visitor, only_values ? Value{data} : arg_case(data));
    };

    try
    {
        ranges::for_each(args, update);
    }
    catch(const NeedHelpException& ex)
    {
        error_handler(ex);
    }
    catch(const FewArgumentsException& ex)
    {
        error_handler(ex);
    }
    catch(const KeyException<char>& ex)
    {
        error_handler(ex);
    }
    catch(const KeyException<std::string_view>& ex)
    {
        error_handler(ex);
    }

    std::string_view incomplete_option = visitor.check_opt();
    if(!incomplete_option.empty()) error_handler(incomplete_option);

    if(visitor.check_pos()) error_handler(true);

    return ParseResults(visitor.get_bindings(), options);
}

template <typename... Options, typename ErrorHandler>
auto
parse(const std::tuple<Options...>& options, ArgsRange args, ErrorHandler&& error_handler) noexcept
{
    return parse(options, std::tuple<>{}, args, std::move(error_handler));
}
} // namespace lucid::argparse

namespace fmt
{
namespace internal
{
template <typename IterOut>
constexpr IterOut
copy(IterOut out, const std::string_view word) noexcept
{
    return copy_str<char>(word.cbegin(), word.cend(), out);
}

template <std::size_t nvals, typename IterOut, typename Var>
constexpr IterOut
format_var(IterOut out, const Var& var) noexcept
{
    if constexpr(nvals == 1) { out = copy(out, var); }
    else if constexpr(nvals == -1ul)
    {
        out = format_to(out, FMT_STRING("{}..."), var);
    }
    else
    {
        out = copy(out, var[0]);
        for(std::size_t i = 1; i < nvals; ++i) { out = format_to(out, FMT_STRING(" {}"), var[i]); }
    }
    return out;
}

template <typename Formatter, typename Tuple, std::size_t... Idxs>
constexpr void
for_each(Formatter&& formatter, const Tuple& tpl, std::index_sequence<Idxs...>)
{
    (..., formatter(std::get<Idxs>(tpl)));
}
} // namespace internal

template <char Key, typename Converter, std::size_t nvals>
struct formatter<lucid::argparse::Option<Key, Converter, nvals>>
{
  private:
    char presentation = 's';

  public:
    constexpr auto
    parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();

        if(it != end && (*it == 's' || *it == 'l')) presentation = *it++;

        if(it != end && *it != '}') throw format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    constexpr auto
    format(const lucid::argparse::Option<Key, Converter, nvals>& desc, FormatContext& ctx) const
    {
        auto out = ctx.out();
        if(presentation == 's')
        {
            out = format_to(out, FMT_STRING("-{}"), Key);
            if constexpr(!lucid::argparse::Option<Key, Converter, nvals>::is_flag)
            {
                *out++ = ' ';
                out    = internal::format_var<nvals>(out, desc.var);
            }
        }
        else
        {
            out = format_to(out, FMT_STRING("-{}"), Key);
            if(!desc.keyword.empty()) out = format_to(out, FMT_STRING(", --{} "), desc.keyword);
            out = internal::format_var<nvals>(out, desc.var);
        }
        return out;
    }
};

template <typename Converter, std::size_t nvals>
struct formatter<lucid::argparse::Positional<Converter, nvals>>
{
    constexpr auto
    parse(format_parse_context& ctx) const
    {
        auto it = ctx.begin();
        return it++;
    }

    template <typename FormatContext>
    constexpr auto
    format(const lucid::argparse::Positional<Converter, nvals>& desc, FormatContext& ctx) const
    {
        return internal::format_var<nvals>(ctx.out(), desc.var);
    }
};

template <typename... Options>
struct formatter<lucid::argparse::detail::fmt_options<std::tuple<Options...>>>
{
    using Data = lucid::argparse::detail::fmt_options<std::tuple<Options...>>;

  private:
    char presentation = 's';

  public:
    constexpr auto
    parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();

        if(it != end && (*it == 's' || *it == 'l')) presentation = *it++;

        if(it != end && *it != '}') throw format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    constexpr auto
    format(const Data& desc, FormatContext& ctx) const
    {
        constexpr auto indices = std::index_sequence_for<Options...>{};
        const auto&    options = desc.value;

        auto out = ctx.out();
        if(presentation == 's')
        {
            out = internal::copy(out, "[-h");

            if constexpr((false || ... || Options::is_flag))
                internal::for_each(
                    [&out](const auto& option) {
                        using OptionType = std::decay_t<decltype(option)>;
                        if constexpr(OptionType::is_flag) *out++ = OptionType::key;
                    },
                    options,
                    indices);

            *out++ = ']';

            internal::for_each(
                [&out](const auto& option) {
                    using OptionType = std::decay_t<decltype(option)>;
                    if constexpr(!OptionType::is_flag)
                        out = format_to(out, FMT_STRING(" [{:s}]"), option);
                },
                options,
                indices);
        }
        else
        {
            internal::for_each(
                [&out](const auto& option) {
                    out = format_to(out, FMT_STRING("\n[{:l}\t"), option, option.doc);
                },
                options,
                indices);
        }
        return out;
    }
};

template <typename... Options, typename... Positionals>
struct formatter<
    lucid::argparse::detail::fmt_data<std::tuple<Options...>, std::tuple<Positionals...>>>
{
    using Desc =
        lucid::argparse::detail::fmt_data<std::tuple<Options...>, std::tuple<Positionals...>>;

    constexpr auto
    parse(format_parse_context& ctx) const
    {
        auto it = ctx.begin();
        return it++;
    }

    template <typename FormatContext>
    constexpr auto
    format(const Desc& desc, FormatContext& ctx) const
    {
        auto out = ctx.out();

        out = format_to(out, FMT_STRING("Usage: {} {:s}"), desc.program_name, desc.options);

        return out;
    }
};
} // namespace fmt

namespace lucid::argparse
{
template <typename... Options, typename... Positionals>
void
show_help(std::string_view                  program_name,
          const std::tuple<Options...>&     options,
          const std::tuple<Positionals...>& positionals,
          FILE*                             file = stderr) noexcept
{
    fmt::print(file, FMT_STRING("{}\n"), detail::fmt_data(program_name, options, positionals));
}

template <typename Options, typename Positionals = std::tuple<>>
class StandardErrorHandler
{
    static constexpr std::string_view error_head = "Argument Parsing Error";

    static std::string_view
    make_program_name(ArgsRange args) noexcept
    {
        const std::string_view argv0{*args.read()};
        const std::size_t      slash_pos{argv0.rfind('/')};
        return slash_pos == std::string_view::npos ? argv0 : argv0.substr(slash_pos + 1);
    }

    const Options&     options;
    const Positionals& positionals;
    std::string_view   program_name;

    void
    help(FILE* file = stderr) const noexcept
    {
        show_help(program_name, options, positionals, file);
    };

  public:
    StandardErrorHandler(ArgsRange          args,
                         const Options&     _options,
                         const Positionals& _positionals = Positionals{}) :
        options(_options),
        positionals(_positionals), program_name(make_program_name(args))
    {
    }

    StandardErrorHandler()                            = delete;
    StandardErrorHandler(const StandardErrorHandler&) = delete;
    StandardErrorHandler(StandardErrorHandler&&)      = delete;
    StandardErrorHandler&
    operator=(const StandardErrorHandler&) = delete;

    void
    operator()(const NeedHelpException&) const noexcept
    {
        help(stdout);
        exit(0);
    }

    void
    operator()(const FewArgumentsException& ex) const noexcept
    {
        fmt::print(stderr,
                   FMT_STRING("{}: too few arguments for parameter \"{}\"\n"),
                   error_head,
                   ex.keyword);
        help(stderr);
        exit(1);
    }

    template <typename Key>
    void
    operator()(const KeyException<Key>& ex) const noexcept
    {
        fmt::print(stderr, FMT_STRING("{}: unknown key \"{}\"\n"), error_head, ex.value);
        help(stderr);
        exit(1);
    }

    void
    operator()(const std::string_view problem_key) const noexcept
    {
        fmt::print(stderr,
                   FMT_STRING("{}: too few arguments for parameter \"{}\"\nUsage:\n"),
                   error_head,
                   problem_key);
        help(stderr);
    }

    void
    operator()(bool) const noexcept
    {
        fmt::print(stderr, FMT_STRING("{}: too few positional arguments\nUsage:\n"), error_head);
        help();
        exit(1);
    }
};
} // namespace lucid::argparse
