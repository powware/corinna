#ifndef __TASK_H__
#define __TASK_H__

#include <cassert>
#include <chrono>
#include <coroutine>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <variant>

#include "executor.h"

namespace corinna
{
    template <typename T>
    struct task_promise;

    template <typename T = void>
    class [[nodiscard]] task
    {
    public:
        using promise_type = task_promise<T>;

        using coroutine_handle = std::coroutine_handle<promise_type>;

        explicit task(coroutine_handle coroutine) noexcept : coroutine_(coroutine) {}

        // delete copy and move assignment
        task(const task &) = delete;
        auto operator=(const task &) = delete;
        auto operator=(task &&) = delete;

        task(task &&rhs) noexcept : coroutine_(std::exchange(rhs.coroutine_, {})) {}

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

            explicit awaiter(coroutine_handle coroutine) noexcept : coroutine_(coroutine) {}

            bool await_ready() const noexcept { return false; }

            // remember the suspended coroutine, so it can be resumed in the final_awaiter
            // symmetric transfer by returning the coroutine to be resumed
            template <typename Promise>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> caller)
            {
                coroutine_.promise().caller_ = caller;

                auto executor = caller.promise().executor_;

                coroutine_.promise().executor_ = executor;

                executor->add(executable([]()
                                         { return true; },
                                         coroutine_));

                return executor->next();
            }

            decltype(auto) await_resume()
            {
                return coroutine_.promise().result();
            };
        };

        template <bool False = false>
        auto operator co_await() & { static_assert(False, "co_await is forbidden for lvalues"); }

        auto operator co_await() &&noexcept { return awaiter(coroutine_); }

        decltype(auto) execute(executor &executor) &&
        {
            coroutine_.promise().executor_ = &executor;
            coroutine_.resume();

            return coroutine_.promise().result();
        }

        // private:
        coroutine_handle coroutine_;
    };

    template <typename T>
    struct task_promise_base
    {
        using coroutine_handle = std::coroutine_handle<typename task<T>::promise_type>;

        std::coroutine_handle<> caller_;

        executor *executor_;

        struct final_awaiter
        {
            bool await_ready() const noexcept { return false; }

            // symmetric transfer by returning the coroutine to be resumed
            std::coroutine_handle<> await_suspend(coroutine_handle callee) noexcept
            {
                auto caller = callee.promise().caller_;

                auto executor = callee.promise().executor_;

                if (caller)
                {
                    executor->add(executable([]()
                                             { return true; },
                                             caller));
                }

                return executor->next();
            }

            // resuming the final suspend causes the coroutine to be destroyed by walking off the body as well as from the task destructor
            void await_resume() noexcept { std::terminate(); };
        };

        std::suspend_always initial_suspend() noexcept { return {}; }

        final_awaiter final_suspend() noexcept { return {}; }
    };

    template <typename T>
    struct task_promise : task_promise_base<T>
    {
        static constexpr auto is_result_lvalue_reference = std::is_lvalue_reference_v<T>;

        using storage_type = std::conditional_t<is_result_lvalue_reference, std::reference_wrapper<std::remove_reference_t<T>>, std::decay_t<T>>;

        using typename task_promise_base<T>::coroutine_handle;

        std::variant<std::optional<storage_type>, std::exception_ptr> result_{std::nullopt};

        auto get_return_object() { return task<T>(coroutine_handle::from_promise(*this)); }

        void unhandled_exception() noexcept
        {
            result_ = std::current_exception();
        }

        void return_value(auto &&value)
        {
            if constexpr (is_result_lvalue_reference)
            {
                result_ = std::ref(value);
            }
            else
            {
                result_ = std::forward<decltype(value)>(value);
            }
        };

        decltype(auto) result()
        {
            if (std::holds_alternative<std::exception_ptr>(result_))
            {
                std::rethrow_exception(std::get<std::exception_ptr>(result_));
            }

            // co_return missing in couroutine returning task<T> where T is of non-void type
            assert(std::get<std::optional<storage_type>>(result_));

            if constexpr (is_result_lvalue_reference)
            {
                // return reference_wrapper content as reference
                return (*std::get<std::optional<storage_type>>(result_)).get();
            }
            else
            {
                // usecase for C++23 auto(std::move(*std::get<std::optional<storage_type>>(result_))) as decay_copy
                return std::decay_t<T>(std::move(*std::get<std::optional<storage_type>>(result_)));
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

        void return_void() const noexcept {};

        void result()
        {
            if (exception_)
            {
                std::rethrow_exception(exception_);
            }
        }
    };

    template <typename Task>
    decltype(auto) sync_await(Task &&task)
    {
        executor e;
        return std::move(task).execute(e);
    }

    template <typename Task, typename... Tasks>
    void async_await_impl(Task &&task, Tasks &&...tasks)
    {
        executor e;
        ((tasks.coroutine_.promise().executor_ = &e), ...);
        (e.add(executable([]()
                          { return true; },
                          std::forward<Tasks>(tasks).coroutine_)),
         ...);
        std::move(task).execute(e);
    }

    template <typename... Tasks>
    void async_await(Tasks &&...tasks)
    {
        async_await_impl(std::forward<Tasks>(tasks)...);
    }

    namespace this_coroutine
    {
        template <typename Clock, typename Duration>
        struct suspend_awaiter
        {
            using time_point_type = std::chrono::time_point<Clock, Duration>;

            suspend_awaiter(const time_point_type &time) : resume_point_(time) {}

            bool await_ready() { return Clock::now() >= resume_point_; }

            template <typename Promise>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> caller)
            {
                auto executor_ = caller.promise().executor_;
                executor_->add(executable([this]()
                                          { return await_ready(); },
                                          caller));
                return executor_->next();
            }

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

#endif // __TASK_H__