#include <chrono>
#include <iostream>

#include <corinna/corinna.h>

using namespace std::chrono_literals;

corinna::task<> SuspendExample()
{
    auto start = std::chrono::steady_clock::now();
    std::cout << "Start!\n";
    co_await corinna::suspend_for(1s);
    auto end = std::chrono::steady_clock::now();
    std::cout << "End! (" << std::chrono::duration_cast<std::chrono::seconds>(start - end).count() << "s)\n";
    co_return;
}

int main()
{
    corinna::sync_await(SuspendExample());
}
