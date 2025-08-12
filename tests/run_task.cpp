#include "../include/Task.hpp"
#include "../include/utils/Function.hpp"

#include <iostream>

auto coro_task() -> Crotine::Task<int>
{
    std::cout << "Coroutine task started\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    co_return 42;
}

auto non_coro_task() -> int
{
    std::cout << "Non-coroutine task started\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 52;
}

auto coro_param_task(int a, int b) -> Crotine::Task<int>
{
    std::cout << "Coroutine task with parameters started\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    co_return a + b;
}

auto non_coro_param_task(int a, int b) -> int
{
    std::cout << "Non-coroutine task with parameters started\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return a + b;
}

int main()
{
    auto tsk1 = Crotine::RunTask(coro_task);
    auto tsk2 = Crotine::RunTask(non_coro_task);

    auto tsk3 = Crotine::RunTask(coro_param_task, 10, 20);
    auto tsk4 = Crotine::RunTask(non_coro_param_task, 30, 40);

    std::cout << "Coroutine task result: " << tsk1.getPromise().getWaitedValue() << "\n";
    std::cout << "Non-coroutine task result: " << tsk2.getPromise().getWaitedValue() << "\n";

    std::cout << "Coroutine task with parameters result: " << tsk3.getPromise().getWaitedValue() << "\n";
    std::cout << "Non-coroutine task with parameters result: " << tsk4.getPromise().getWaitedValue() << "\n";
    return 0;
}