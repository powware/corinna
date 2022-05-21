#include <iostream>

#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/async_latch.hpp>

using namespace cppcoro;

task<void> SubTask(auto &latch)
{
    latch.count_down();
    std::cout << "-> SubTask" << std::endl;
    co_await latch;
    std::cout << "<- SubTask" << std::endl;
}

task<void> Task(auto &latch)
{
    std::cout << "-> Task" << std::endl;
    co_await SubTask(latch);
    std::cout << "<- Task" << std::endl;
}

int main()
{
    async_latch latch(3);

    sync_wait(when_all_ready(Task(latch), Task(latch), Task(latch)));
}