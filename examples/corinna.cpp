#include <chrono>
#include <iostream>
#include <stdexcept>

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

corinna::task<int> Inner2()
{
    co_return 42;
}

corinna::task<int> ReturnValueExample()
{
    auto value = co_await Inner2();
    std::cout << "number: " << value << std::endl;
    co_return value;
}

corinna::task<int> ExceptionExample()
{
    if (co_await Inner2() == 42)
    {
        throw std::logic_error("bad number");
    }

    co_return 0;
}

// int global;
// corinna::task<int &> Inner3()
// {
//     global = 12;
//     co_return global;
// }

// corinna::task<int &> ReturnReferenceExample()
// {
//     auto &ref = co_await Inner3();
//     std::cout << "number: " << ref << std::endl;
//     ++ref;
//     co_return global;
// }

int main()
{
    corinna::sync_await(HelloWorldExample());

    auto value = corinna::sync_await(ReturnValueExample());
    std::cout << "number still is: " << value << std::endl;

    try
    {
        auto unused = corinna::sync_await(ExceptionExample());
        std::cout << "unused is: " << unused << std::endl;
    }
    catch (const std::logic_error &exception)
    {
        std::cout << "exception: " << exception.what() << std::endl;
    }
    // auto return_value = corinna::sync_await(ReturnValueExample());
}
