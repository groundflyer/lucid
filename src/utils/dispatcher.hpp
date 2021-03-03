// -*- C++ -*-
/// @file dispatcher.hpp
/// @brief Contains implementation of task dispatcher.

#include <utils/steady_tuple.hpp>
#include <utils/tuple.hpp>
#include <utils/typelist.hpp>

#include <atomic>
#include <optional>
#include <thread>
#include <vector>

#include <concurrentqueue.h>

namespace lucid
{
namespace detail
{
// Hiding this until stable GCC with concepts support.
// template <typename Fn, typename Ret, TypeList Inputs>
// struct signature_helper;

// template <typename Fn, typename Ret, typename... Args>
// struct signature_helper<Fn, Ret, type_sequence<Args...>>
// {
//     static constexpr bool value = std::is_nothrow_invocable_r_v<Ret, Fn, Args...>;
// };

// template <typename T, TypeList Signature>
// struct is_signature_valid
// {
//     using inputs                = tl::take_t<tl::length_v<Signature> - 1, Signature>;
//     static constexpr bool value = signature_helper<const T, tl::last<Signature>, inputs>::value;
// };

// template <typename T>
// concept Dispatchable = requires
// {
//     std::is_default_constructible_v<T>;
//     std::is_copy_assignable_v<T>;

//     typename T::signature;
//     is_typelist_v<typename T::signature>;
//     // At least one argument and corresponding return type
//     tl::length_v<typename T::signature> >= 2;
//     detail::is_signature_valid<T, typename T::signature>::value;
// };

template <typename InputTypes>
using maybe_tuple = std::conditional_t<typelist::length_v<InputTypes> == 1,
                                       typelist::head<InputTypes>,
                                       typelist::as_tuple<InputTypes>>;

template <typename Signature>
struct signature_helper
{
    using input_types = typelist::take_t<typelist::length_v<Signature> - 1, Signature>;
    using input_type  = maybe_tuple<input_types>;
    using output_type = typelist::last<Signature>;
};
} // namespace detail

template <typename T>
using cqueue = moodycamel::ConcurrentQueue<T>;

/// @brief Lock-free typefull multi-threaded task dispatcher.
template <typename... Fns>
class Dispatcher
{
    using InputTypes =
        type_sequence<typename detail::signature_helper<typename Fns::signature>::input_type...>;
    using OutputTypes =
        type_sequence<typename detail::signature_helper<typename Fns::signature>::output_type...>;
    using InputPool  = typelist::repack_t<steady_tuple, typelist::map_t<cqueue, InputTypes>>;
    using OutputPool = typelist::repack_t<steady_tuple, typelist::map_t<cqueue, OutputTypes>>;
    using FnTypes    = type_sequence<Fns...>;
    using FnTuple    = typelist::as_tuple<FnTypes>;

    template <typename F>
    static constexpr std::size_t func_index = typelist::find_v<F, FnTypes>;

    struct ThreadWorker
    {
        std::atomic_bool& active_flag;
        const FnTuple&    funcs;
        InputPool&        in_pool;
        OutputPool&       out_pool;

        template <typename Func>
        void
        run_task(const Func& func) noexcept
        {
            constexpr std::size_t idx       = func_index<Func>;
            auto&                 in_queue  = in_pool.template get<idx>();
            auto&                 out_queue = out_pool.template get<idx>();

            typelist::elem_t<idx, InputTypes> inputs;
            if(in_queue.try_dequeue(inputs))
            {
                const auto result = maybe_apply(func, inputs);
                while(!out_queue.try_enqueue(std::move(result)) &&
                      active_flag.load(std::memory_order_relaxed))
                {
                }
            }
        }

        void
        operator()() noexcept
        {
            while(active_flag.load(std::memory_order_relaxed))
            {
                std::apply([&](const auto&... func) { (..., run_task(func)); }, funcs);
            }
        }
    };

    const FnTuple fns;
    InputPool     in_pool;
    OutputPool    out_pool;

    std::atomic_bool         active_flag{true};
    std::vector<std::thread> threads;

  public:
    Dispatcher(const std::size_t queue_size,
               const unsigned    num_threads,
               const Fns&... _fns) noexcept :
        fns(_fns...),
        in_pool(uniform_init{}, queue_size), out_pool(uniform_init{}, queue_size)
    {
        threads.reserve(num_threads);
        for(unsigned t_idx = 0; t_idx < num_threads; ++t_idx)
            threads.emplace_back(ThreadWorker{active_flag, fns, in_pool, out_pool});
    }

    Dispatcher()                  = delete;
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher(Dispatcher&&)      = delete;
    Dispatcher&
    operator=(const Dispatcher&) = delete;

    template <typename Func, typename... Ts>
    bool
    try_submit(const Ts&... inputs) noexcept
    {
        constexpr std::size_t idx = func_index<Func>;
        static_assert(idx != -1ul, "Function not defined in dispatcher");

        auto& queue = in_pool.template get<idx>();
        if constexpr(sizeof...(Ts) == 1)
            return queue.try_enqueue(inputs...);
        else
            return queue.try_enqueue(std::tuple{inputs...});
    }

    template <typename... Ts>
    bool
    try_submit(const Ts&... inputs) noexcept
    {
        using input_type          = detail::maybe_tuple<type_sequence<Ts...>>;
        constexpr std::size_t itc = typelist::count_v<input_type, InputTypes>;
        static_assert(itc < 2, "Ambigious dispatch. Use explicit overload.");

        constexpr std::size_t idx = typelist::find_v<input_type, InputTypes>;
        using fn_type             = typelist::elem_t<idx, FnTypes>;
        return try_submit<fn_type>(inputs...);
    }

    template <typename Func>
    std::optional<typename typelist::elem_t<func_index<Func>, OutputTypes>>
    fetch_result() noexcept
    {
        constexpr std::size_t idx = func_index<Func>;
        using result_type         = typelist::elem_t<idx, OutputTypes>;
        auto&       queue         = out_pool.template get<idx>();
        result_type result;
        if(queue.try_dequeue(result)) return result;

        return std::optional<result_type>{};
    }

    ~Dispatcher() noexcept
    {
        active_flag.store(false, std::memory_order_relaxed);
        for(std::thread& t: threads) t.join();
    }
};

} // namespace lucid
