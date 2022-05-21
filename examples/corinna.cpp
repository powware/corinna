#include <chrono>
#include <iostream>
#include <stdexcept>

#include <corinna/task.h>

using namespace std::chrono_literals;

corinna::task<> HelloWorldInner()
{
    std::cout << ", ";
    co_return;
}

corinna::task<> HelloWorldExample()
{
    std::cout << "Hello";
    co_await HelloWorldInner();
    std::cout << "World!" << std::endl;
}

corinna::task<int> ReturnValueInner()
{
    co_return 42;
}

corinna::task<int> ReturnValueExample()
{
    auto value = co_await ReturnValueInner();
    std::cout << "number: " << value << std::endl;
    co_return value;
}

corinna::task<int> ExceptionExample()
{

    throw std::logic_error("error");

    // unreachable
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

corinna::task<Expensive> ExpensiveInner()
{
    Expensive expensive{};
    co_return expensive;
}

corinna::task<Expensive> ReturnExpensiveValueExample()
{
    auto expensive = co_await ExpensiveInner();
    co_return expensive;
}

int global;
corinna::task<int &> ReferenceInner()
{
    global = 12;
    std::cout << "global: " << global << std::endl;
    co_return global;
}

corinna::task<int &> ReturnReferenceExample()
{
    auto &ref = co_await ReferenceInner();
    ++ref;
    co_return ref;
}

corinna::task<> SuspendExample()
{
    std::cout << "Suspending!" << std::endl;
    co_await corinna::this_coroutine::suspend_for(1s);
    std::cout << "Awake from suspend!" << std::endl;
    co_return;
}

corinna::task<> SuspendShortlyExample()
{
    for (auto i = 0; i < 10; ++i)
    {
        std::cout << "Suspending shortly!" << std::endl;
        co_await corinna::this_coroutine::suspend_for(100ms);
        std::cout << "Awake from short suspend!" << std::endl;
    }

    co_return;
}

// corinna::task<> HandleConnection(corinna::connection connection)
// {
//     auto data = co_await connection.receive();
// }

// corinna::task<> f()
// {
//     corinna::socket socket();
//     for (auto connection : co_await socket)
//     {
//     }
// }

int main()
{
    corinna::sync_await(HelloWorldExample());

    {
        auto value = corinna::sync_await(ReturnValueExample());
        std::cout << "number still is: " << value << std::endl;
    }

    try
    {
        auto unreachable = corinna::sync_await(ExceptionExample());
        std::cout << "unreachable is: " << unreachable << std::endl;
    }
    catch (const std::logic_error &exception)
    {
        std::cout << "exception: " << exception.what() << std::endl;
    }

    {
        auto expensive = corinna::sync_await(ReturnExpensiveValueExample());
    }

    {
        auto &ref = corinna::sync_await(ReturnReferenceExample());
        ++ref;
        std::cout << "global is now: " << global << std::endl;
    }

    auto start = std::chrono::steady_clock::now();
    corinna::async_await(SuspendExample(), SuspendShortlyExample(), SuspendExample(), SuspendExample());
    auto end = std::chrono::steady_clock::now();
    std::cout << "async_await() took " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "s" << std::endl;
}