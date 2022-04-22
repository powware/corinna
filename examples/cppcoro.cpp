#include <iostream>

#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>

using namespace cppcoro;

task<std::string> makeTask()
{
    co_return "foo";
}

task<std::string> makeTask2()
{
    std::cout << co_await makeTask() << std::endl;
    std::cout << co_await makeTask() << std::endl;
    std::cout << co_await makeTask() << std::endl;
    std::cout << co_await makeTask() << std::endl;
    std::cout << co_await makeTask() << std::endl;
    co_return "foo2";
}

int main()
{

    // start the lazy task and wait until it completes
    std::cout << sync_wait(makeTask2()) << std::endl; // -> "foo"
}