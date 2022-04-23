#include <chrono>
#include <coroutine>
#include <memory>
#include <thread>
#include <utility>

namespace corinna
{
    template <typename T = void>
    class task;

    template <typename T>
    struct [[nodiscard]] task
    {
        using value_type = T;

        struct promise_type
        {
            constexpr auto get_return_object() { return task(std::coroutine_handle<promise_type>::from_promise(*this)); }

            constexpr std::suspend_always initial_suspend() noexcept { return {}; }

            constexpr std::suspend_never final_suspend() noexcept { return {}; }

            constexpr void unhandled_exception() {}

            constexpr void return_void() {}
        };

        using coroutine_type = std::coroutine_handle<promise_type>;

        task(const task &) = delete;

        constexpr task(task &&rhs) noexcept : coroutine_(std::exchange(rhs.coroutine_, {})) {}

        explicit constexpr task(coroutine_type coroutine) noexcept : coroutine_(coroutine) {}

        ~task()
        {
            if (coroutine_)
            {
                coroutine_.destroy();
            }
        }

        value_type result()
        {
            // if ()
            // {
            //     throw;
            // }

            if constexpr (!std::is_same_v<value_type, void>)
            {
                return {};
            }
        }

        coroutine_type coroutine_;
    };

    // template <typename T>
    // task<T> task_promise<T>::get_return_object() { return task<T>(std::coroutine_handle<task_promise<T>>); }

    template <typename TODO_AWAITABLE>
    auto sync_await(TODO_AWAITABLE &&task__)
    {
        TODO_AWAITABLE task(std::move(task__));
        while (!task.coroutine_.done())
        {
            task.coroutine_.resume();
        }
    }

    namespace this_coroutine
    {
        template <typename Clock, typename Duration>
        struct suspend_awaiter
        {
            using time_point_type = std::chrono::time_point<Clock, Duration>;

            suspend_awaiter(const time_point_type &time) : resume_point_(time) {}

            bool await_ready() { return Clock::now() >= resume_point_; }

            void await_suspend(std::coroutine_handle<>) {}

            void await_resume()
            {
                std::this_thread::sleep_until(resume_point_);
            }

            time_point_type resume_point_;
        };

        template <typename Clock, typename Duration>
        auto suspend_until(const std::chrono::time_point<Clock, Duration> &time)
        {
            return suspend_awaiter(time);
        }

        template <typename Rep, typename Period>
        inline auto suspend_for(std::chrono::duration<Rep, Period> duration)
        {
            return suspend_until(std::chrono::steady_clock::now() + duration);
        }
    }

}