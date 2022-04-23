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

corinna::task<> TrivialExample()
{
    std::cout << "Start!" << std::endl;
    co_await corinna::this_coroutine::suspend_for(1s);
    std::cout << "End!" << std::endl;
}

int main()
{
    corinna::sync_await(TrivialExample());
    // corinna::sync_await(SuspendExample());
}
