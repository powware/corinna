#include <cassert>
#include <chrono>
#include <coroutine>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <variant>

namespace corinna
{

    template <typename T>
    struct task_promise;

    template <typename T = void>
    struct [[nodiscard]] task
    {
        using promise_type = task_promise<T>;

        using coroutine_handle = std::coroutine_handle<promise_type>;

        coroutine_handle coroutine_;

        explicit constexpr task(coroutine_handle coroutine) noexcept : coroutine_(coroutine) {}

        task(const task &) = delete;
        auto operator=(const task &) = delete;

        constexpr task(task &&rhs) noexcept : coroutine_(std::exchange(rhs.coroutine_, {})) {}

        constexpr auto &operator=(task &&rhs) noexcept { std::swap(coroutine_, rhs.coroutine_); }

        ~task()
        {
            if (coroutine_)
            {
                coroutine_.destroy();
            }
        }

        struct awaiter
        {
            coroutine_handle coroutine_;

            explicit constexpr awaiter(coroutine_handle coroutine) : coroutine_(coroutine) {}

            constexpr bool await_ready() noexcept { return false; }

            constexpr void await_suspend(std::coroutine_handle<> continuation)
            {
                coroutine_.promise().continuation_ = continuation;

                if (!coroutine_)
                {
                    throw "error";
                }
                coroutine_.resume();
            }

            constexpr T await_resume()
            {
                return coroutine_.promise().result();
            };
        };

        template <bool False = false>
        auto operator co_await() & { static_assert(False, "co_await is forbidden for lvalues"); }

        constexpr auto operator co_await() &&noexcept { return awaiter(coroutine_); }
    };

    template <typename T>
    struct task_promise_base
    {
        using coroutine_handle = std::coroutine_handle<typename task<T>::promise_type>;

        std::coroutine_handle<> continuation_;

        struct final_awaiter
        {
            constexpr bool await_ready() noexcept { return false; }

            void await_suspend(coroutine_handle coroutine) noexcept
            {
                auto &continuation = coroutine.promise().continuation_;
                if (!continuation)
                {
                    return;
                }
                continuation.resume();
            }

            void await_resume() noexcept { std::terminate(); };
        };

        friend final_awaiter;

        constexpr std::suspend_always initial_suspend() noexcept { return {}; }

        constexpr final_awaiter final_suspend() noexcept { return {}; }
    };

    template <typename T>
    struct task_promise : task_promise_base<T>
    {
        using typename task_promise_base<T>::coroutine_handle;

        std::variant<std::optional<T>, std::exception_ptr> result_{std::nullopt};

        auto get_return_object() { return task<T>(coroutine_handle::from_promise(*this)); }

        void unhandled_exception() noexcept
        {
            result_ = std::current_exception();
        }

        void return_value(T value) { result_ = value; };

        T result()
        {
            if constexpr (!std::is_same_v<T, void>)
            {
                if (std::holds_alternative<std::exception_ptr>(result_))
                {
                    std::rethrow_exception(std::get<std::exception_ptr>(result_));
                }

                assert(std::get<std::optional<T>>(result_));

                return *std::get<std::optional<T>>(result_);
            }
        }
    };

    template <>
    struct task_promise<void> : task_promise_base<void>
    {
        using typename task_promise_base<void>::coroutine_handle;

        std::exception_ptr exception_{nullptr};

        auto get_return_object() { return task<void>(coroutine_handle::from_promise(*this)); }

        void unhandled_exception() noexcept
        {
            exception_ = std::current_exception();
        }

        constexpr void return_void() noexcept {};

        void result()
        {
            if (exception_)
            {
                std::rethrow_exception(exception_);
            }
        }
    };

    template <typename Task>
    auto sync_await(Task &&task)
    {
        auto sync_task = [](Task &&task) -> Task
        { co_return co_await std::move(task); }(std::move(task));

        sync_task.coroutine_.resume();
        return sync_task.coroutine_.promise().result();
    }

    //     namespace this_coroutine
    //     {
    //         template <typename Clock, typename Duration>
    //         struct suspend_awaiter
    //         {
    //             using time_point_type = std::chrono::time_point<Clock, Duration>;

    //             suspend_awaiter(const time_point_type &time) : resume_point_(time) {}

    //             bool await_ready() { return Clock::now() >= resume_point_; }

    //             void await_suspend(std::coroutine_handle<>) {}

    //             void await_resume()
    //             {
    //                 std::this_thread::sleep_until(resume_point_);
    //             }

    //             time_point_type resume_point_;
    //         };

    //         template <typename Clock, typename Duration>
    //         auto suspend_until(const std::chrono::time_point<Clock, Duration> &time)
    //         {
    //             return suspend_awaiter(time);
    //         }

    //         template <typename Rep, typename Period>
    //         inline auto suspend_for(std::chrono::duration<Rep, Period> duration)
    //         {
    //             return suspend_until(std::chrono::steady_clock::now() + duration);
    //         }
    //     }
    // }
}