#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include <coroutine>
#include <deque>
#include <functional>
#include <tuple>

struct executable
{
    std::function<bool()> is_ready;
    std::coroutine_handle<> coroutine;
};

struct executor
{
    std::deque<executable> executables_;

    void add(executable &&executable)
    {
        executables_.push_back(std::move(executable));
    }

    std::coroutine_handle<> next()
    {
        if (executables_.empty())
        {
            return std::noop_coroutine();
        }

        while (true)
        {
            auto &&executable = std::move(executables_.front());
            executables_.pop_front();

            if (executable.is_ready())
            {
                return executable.coroutine;
            }
            else
            {
                executables_.push_back(std::move(executable));
            }
        }
    }
};

#endif // __EXECUTOR_H__