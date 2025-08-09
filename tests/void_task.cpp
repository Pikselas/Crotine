#include <string>
#include <iostream>
#include "../include/Task.hpp"

int main()
{
    int x = 10;


    auto task1 = [&]() -> Crotine::Task<int>
    {
        std::cout << "Task 1 started with x = " << x << "\n";
        ++x;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task 1 completed with x = " << x << "\n";
        co_return x;
    };

    auto task2 = [&]() -> Crotine::Task<void>
    {
        auto tsk = task1();
        tsk.execute_async();
        std::cout << "Task 2 started with x = " << x << "\n";
        auto val = co_await tsk;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task completed with x = " << x << "\n";
    }();

    task2.execute_async();
    task2.getPromise().Wait();
    std::cout << "Main thread continues with x = " << x << "\n";
    return 0;
}