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
template <typename T>
using cqueue = moodycamel::ConcurrentQueue<T>;

template <typename... Tasks>
class Dispatcher
{
    using TaskTypeList = typelist<Tasks...>;
    template <typename T>
    using no_args         = std::invoke_result_t<T>;
    using ResultTypeList  = typename TaskTypeList::template map<no_args, typelist>;
    using TaskQueuePool   = typename TaskTypeList::template map<cqueue, steady_tuple>;
    using ResultQueuePool = typename ResultTypeList::template map<cqueue, steady_tuple>;

    template <typename Task>
    static constexpr std::size_t
    result_index() noexcept
    {
        return TaskTypeList::template index<Task>();
    }

    template <typename Task>
    using ResultQueue = cqueue<typename ResultTypeList::template at<result_index<Task>()>>;

    template <typename T>
    struct result_helper;

    template <typename ResultType>
    struct result_helper<cqueue<ResultType>>
    {
        using type = ResultType;
    };

    struct ThreadWorker
    {
        std::atomic_bool& active_flag;
        TaskQueuePool&    task_queue_pool;
        ResultQueuePool&  result_queue_pool;

        template <typename Task>
        void
        run_task(cqueue<Task>& task_queue) noexcept
        {
            auto& result_queue = result_queue_pool.template get<result_index<Task>()>();

            Task task;
            if(task_queue.try_dequeue(task))
            {
                const auto result = task();
                while(!result_queue.try_enqueue(std::move(result)) &&
                      active_flag.load(std::memory_order_relaxed))
                {}
            }
        }

        void
        operator()() noexcept
        {
            while(active_flag.load(std::memory_order_relaxed))
            {
                lucid::apply([&](auto&... task_queue) { (..., run_task(task_queue)); },
                             task_queue_pool);
            }
        }
    };

    TaskQueuePool   task_queue_pool;
    ResultQueuePool result_queue_pool;

    std::atomic_bool         active_flag{true};
    std::vector<std::thread> threads;
    std::atomic_size_t       count{0};

  public:
    Dispatcher(const std::size_t queue_size  = 10000,
               const unsigned    num_threads = std::thread::hardware_concurrency()) noexcept :
        task_queue_pool(uniform_init{}, queue_size),
        result_queue_pool(uniform_init{}, queue_size)
    {
        threads.reserve(num_threads);
        for(unsigned t_idx = 0; t_idx < num_threads; ++t_idx)
            threads.emplace_back(ThreadWorker{active_flag, task_queue_pool, result_queue_pool});
    }

    template <typename Task>
    bool
    try_submit(Task&& task) noexcept
    {
        cqueue<std::decay_t<Task>>& task_queue =
            task_queue_pool.template get<cqueue<std::decay_t<Task>>>();
        return task_queue.try_enqueue(std::move(task));
    }

    template <typename Task>
    std::optional<typename result_helper<ResultQueue<Task>>::type>
    fetch_result() noexcept
    {
        using ResultType        = typename result_helper<ResultQueue<Task>>::type;
        auto&      result_queue = result_queue_pool.template get<result_index<Task>()>();
        ResultType result;
        if(result_queue.try_dequeue(result)) return result;

        return std::optional<ResultType>{};
    }

    ~Dispatcher() noexcept
    {
        active_flag.store(false, std::memory_order_relaxed);
        for(std::thread& t: threads) t.join();
    }
};

} // namespace lucid
