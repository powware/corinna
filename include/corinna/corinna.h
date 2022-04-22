#include <chrono>
#include <coroutine>
#include <memory>

namespace corinna
{
    template <typename value_type = void>
    struct task
    {
        struct promise_base
        {
            task get_return_object() { return {}; }
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void unhandled_exception() {}
        };

        struct promise_void : promise_base
        {
            void return_void() {}
        };

        struct promise_value : promise_base
        {
            value_type return_value()
            {
                if constexpr (!std::is_same_v<value_type, void>)
                {
                    return {};
                }
            }
        };

        using promise_type = std::conditional_t<std::is_same_v<value_type, void>, promise_void, promise_value>;

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

        std::coroutine_handle<promise_type> coroutine_handle_;
    };

    template <typename TODO_AWAITABLE>
    auto sync_await(TODO_AWAITABLE &&task)
    {
        while (!task.coroutine_handle_.done())
        {
            task.coroutine_handle_.resume();
        }

        task.coroutine_handle_.destroy();
    }

    template <typename Rep, typename Period>
    auto suspend_for(std::chrono::duration<Rep, Period> duration)
    {
        struct awaiter
        {
            using clock = std::chrono::steady_clock;
            awaiter(std::chrono::duration<Rep, Period> duration) : resume_point_(clock::now() + duration) {}
            bool await_ready() { return clock::now() >= resume_point_; }
            void await_suspend(std::coroutine_handle<>) {}
            void await_resume() {}

            decltype(std::chrono::steady_clock::now()) resume_point_;
        };

        return awaiter(duration);
    }

}