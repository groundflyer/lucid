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
struct has_repeating_impl;

template <typename First, typename... Rest>
struct has_repeating_impl<First, Rest...>
{
    static constexpr bool value =
        (false || ... || (First::key == Rest::key)) || has_repeating_impl<Rest...>::value;
};

template <typename Last>
struct has_repeating_impl<Last>
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
argc2array(char* argc[], Converter converter, std::index_sequence<Idxs...>) noexcept
{
    using value_type = std::array<std::decay_t<std::invoke_result_t<Converter, std::string_view>>,
                                  sizeof...(Idxs)>;
    return value_type{converter(argc[Idxs])...};
}
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
        return is_set ? detail::argc2array(words, converter, std::make_index_sequence<nvals>{}) :
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
        return detail::argc2array(words, converter, std::make_index_sequence<nvals>{});
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
struct has_repeating
{
    static constexpr bool value = detail::has_repeating_impl<Options...>::value;
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

template <typename OptBindings, typename PosBindings, typename Options>
struct ParseResults
{
    OptBindings opt_bindings;
    PosBindings pos_bindings;

    constexpr ParseResults(const std::pair<OptBindings, PosBindings>& _bindings, const Options&) :
        opt_bindings(_bindings.first), pos_bindings(_bindings.second)
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
    using OptBindings = std::decay_t<decltype(make_bindings(std::declval<OptDesc>()))>;
    using PosBindings = std::decay_t<decltype(make_bindings(std::declval<PosDesc>()))>;
    static constexpr std::size_t num_pos = std::tuple_size_v<PosDesc>;
    static constexpr bool        has_pos = num_pos > 0ul;

    template <typename Desc, typename Bindings>
    struct State
    {
        const Desc& desc;
        Bindings    bindings;
        std::size_t token;
        std::size_t remain;

        constexpr State(const Desc& _desc, std::size_t _token, std::size_t _remain) :
            desc(_desc), bindings(make_bindings(_desc)), token(_token), remain(_remain)
        {
        }

        State()             = delete;
        State(const State&) = delete;
        State(State&&)      = delete;
        State&
        operator=(const State&) = delete;
    };

    State<OptDesc, OptBindings> opt_state;
    State<PosDesc, PosBindings> pos_state;
    bool                        is_pos = has_pos;

    template <typename Desc>
    static std::size_t
    get_nvals(const std::size_t _token, const Desc& desc)
    {
        return visit_clamped(
            _token,
            [](const auto& desc) noexcept { return std::decay_t<decltype(desc)>::num_vals; },
            desc);
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
        return has_pos && pos_state.token < num_pos;
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
        opt_state(opt_desc, -1ul, 0ul),
        pos_state(pos_desc, 0ul, has_pos ? get_nvals(0ul, pos_desc) : 0ul)
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

    constexpr std::pair<OptBindings, PosBindings>
    get_bindings() const
    {
        return std::pair{opt_state.bindings, pos_state.bindings};
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
            state_update(
                pos_state,
                [&](std::size_t token) { return token >= num_pos; },
                [&](Visitor&) {
                    ++pos_state.token;
                    pos_state.remain = get_nvals(pos_state.token, pos_state.desc);
                    is_pos           = pos_not_set();
                });
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

template <typename... Options, typename... Positionals>
void
show_help(std::string_view                  program_name,
          const std::tuple<Options...>&     options,
          const std::tuple<Positionals...>& positionals,
          FILE*                             file = stderr) noexcept
{
    constexpr std::size_t line_width = 80;
    constexpr bool        has_flags  = (false || ... || Options::is_flag);
    std::size_t           column     = 0;

    auto print = [&](const std::string_view format_string, const auto&... args) noexcept {
        struct
        {
            std::size_t
            operator()(const std::string_view word) const noexcept
            {
                return word.size();
            }

            std::size_t
            operator()(const char) const noexcept
            {
                return 1ul;
            }
        } size_getter;

        fmt::print(file, format_string, args...);
        column += format_string.size() - (sizeof...(args) * 2ul) + (0ul + ... + size_getter(args));
    };

    auto tuple_print = [](const auto& parm_tuple, const auto& elem_printer) noexcept {
        std::apply([&](const auto&... elems) { (..., (elem_printer(elems))); }, parm_tuple);
    };

    print("Usage: {}", program_name);
    const std::size_t margin = column;

    print(" [-h");
    column += 4ul;
    if constexpr(has_flags)
    {
        auto flag_print = [&](const auto& option) noexcept {
            using OptType = std::decay_t<decltype(option)>;
            if constexpr(OptType::is_flag) { print("{}", OptType::key); }
        };

        tuple_print(options, flag_print);
    }
    print("]");

    auto vars_printer = [&](const auto& option) noexcept {
        constexpr std::size_t nvals = std::decay_t<decltype(option)>::num_vals;

        if constexpr(nvals == 1ul)
            print(" {}", option.var);
        else if constexpr(nvals == -1ul)
            print(" {}...", option.var);
        else
            for(const auto& var: option.var) print(" {}", var);
    };

    {
        auto width_checker = [&]() noexcept {
            if(column > line_width)
            {
                fmt::print(file, "\n{:<{}}", ' ', margin);
                column = margin + 1;
            }
        };

        auto opt_print = [&](const auto& option) noexcept {
            using OptType = std::decay_t<decltype(option)>;
            if constexpr(!std::is_same_v<typename OptType::value_type, bool>)
            {
                width_checker();

                print(" [-{}", OptType::key);
                vars_printer(option);
                print("]");
            }
        };

        tuple_print(options, opt_print);

        auto pos_print = [&](const auto& pos) noexcept {
            width_checker();
            vars_printer(pos);
        };

        tuple_print(positionals, pos_print);
    }

    print("\n\nOptions:");

    {
        auto opt_print = [&](const auto& option) noexcept {
            using Type = std::decay_t<decltype(option)>;
            print("\n -{}, --{}", Type::key, option.keyword);

            if constexpr(!Type::is_flag) vars_printer(option);

            print("\t{}", option.doc);
        };

        print("\n -h, --help\tShow this help message and exit.");
        tuple_print(options, opt_print);
    }

    {
        auto pos_print = [&](const auto& pos) noexcept {
            print("\n ");
            vars_printer(pos);
            print("\t{}", pos.doc);
        };
        tuple_print(positionals, pos_print);
    }

    fmt::print(file, "\n");
}

template <typename... Options, typename... Positionals>
auto
parse(const std::tuple<Options...>&     options,
      const std::tuple<Positionals...>& positionals,
      ArgsRange                         args)
{
    static_assert(!has_repeating<Options...>::value, "All keys should be uinique");
    constexpr std::string_view error_head = "Argument Parsing Error";

    const std::string_view argv0{*args.read()};
    const std::size_t      slash_pos = argv0.rfind('/');
    const std::string_view program_name =
        slash_pos == std::string_view::npos ? argv0 : argv0.substr(slash_pos + 1);
    args.next();

    Visitor visitor{options, positionals};
    bool    only_values = false;

    struct need_help
    {
    };
    const auto help = [&](FILE* file = stderr) noexcept {
        show_help(program_name, options, positionals, file);
    };

    auto update = [&](char** data) {
        std::string_view word{*data};

        if(word == "-h" || word == "--help") throw need_help{};

        if(word == "--")
            only_values = true;
        else
            std::visit(visitor, only_values ? Value{data} : arg_case(data));
    };

    try
    {
        ranges::for_each(args, update);
    }
    catch(const need_help&)
    {
        help(stdout);
        exit(0);
    }
    catch(const FewArgumentsException& ex)
    {
        fmt::print(
            stderr, "{}: too few arguments for parameter \"{}\"\nUsage:\n", error_head, ex.keyword);
        help();
        exit(1);
    }

    std::string_view problem_key = visitor.check_opt();
    if(!problem_key.empty())
    {
        fmt::print(stderr,
                   "{}: too few arguments for parameter \"{}\"\nUsage:\n",
                   error_head,
                   problem_key);
        help();
        exit(1);
    }

    if(visitor.check_pos())
    {
        fmt::print(stderr, "{}: too few positional arguments\nUsage:\n", error_head);
        help();
        exit(1);
    }

    return ParseResults(visitor.get_bindings(), options);
}
} // namespace lucid::argparse
