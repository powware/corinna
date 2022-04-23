#include <chrono>
#include <iostream>

#include <corinna/corinna.h>

using namespace std::chrono_literals;

// corinna::task<> SuspendExample()
// {
//     auto start = std::chrono::steady_clock::now();
//     auto end = std::chrono::steady_clock::now();
//     std::cout << "Start! (" << std::chrono::duration_cast<std::chrono::seconds>(start - end).count() << "s)\n";
//     co_await corinna::this_co::suspend_for(1s);
//     end = std::chrono::steady_clock::now();
//     std::cout << "End! (" << std::chrono::duration_cast<std::chrono::seconds>(start - end).count() << "s)\n";
//     co_return;
// }

corinna::task<> Inner()
{
    std::cout << ", ";
    co_return;
}

corinna::task<> HelloWorldExample()
{
    std::cout << "Hello";
    co_await Inner();
    std::cout << "World!" << std::endl;
}

int main()
{
    corinna::sync_await(HelloWorldExample());
    // corinna::sync_await(SuspendExample());
}
