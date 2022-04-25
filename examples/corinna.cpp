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

struct Expensive
{
    bool holds_value = true;
    Expensive()
    {
        std::cout << "Expensive()" << std::endl;
    }

    Expensive(const Expensive &rhs) : holds_value(rhs.holds_value)
    {
        std::cout << "Expensive(const Expensive &rhs)" << std::endl;
    }

    auto &operator=(const Expensive &rhs)
    {
        holds_value = rhs.holds_value;
        std::cout << "auto &operator=(const Expensive &rhs)" << std::endl;
        return *this;
    }

    Expensive(Expensive &&rhs) : holds_value(std::exchange(rhs.holds_value, false))
    {
        std::cout << "Expensive(Expensive &&rhs)" << std::endl;
    }

    auto &operator=(Expensive &&rhs)
    {
        holds_value = std::exchange(rhs.holds_value, false);
        std::cout << "auto &operator=(Expensive &&rhs)" << std::endl;
        return *this;
    }

    ~Expensive()
    {
        if (holds_value)
        {
            std::cout << "~Expensive()" << std::endl;
        }
    }
};

corinna::task<Expensive> ReturnExpensiveValueExample()
{
    Expensive e{};
    co_return std::move(e);
}

int global;
corinna::task<int &> Inner3()
{
    global = 12;
    co_return global;
}

corinna::task<int &> ReturnReferenceExample()
{
    auto &ref = co_await Inner3();
    std::cout << "number: " << ref << std::endl;
    ++ref;
    co_return ref;
}

int main()
{
    corinna::sync_await(HelloWorldExample());

    {
        auto value = corinna::sync_await(ReturnValueExample());
        std::cout << "number still is: " << value << std::endl;
    }

    try
    {
        auto unused = corinna::sync_await(ExceptionExample());
        std::cout << "unused is: " << unused << std::endl;
    }
    catch (const std::logic_error &exception)
    {
        std::cout << "exception: " << exception.what() << std::endl;
    }

    {
        auto expensive = corinna::sync_await(ReturnExpensiveValueExample());
    }

    {
        auto &value = corinna::sync_await(ReturnReferenceExample());
        std::cout << "number is now: " << value << std::endl;
    }
}
