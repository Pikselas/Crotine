#include <string>
#include <iostream>
#include "../include/Task.hpp"

Crotine::Task<int> produceValue()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    co_return 42;
}

Crotine::Task<std::string> produceStrValue()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Producing string value...\n";
    co_return "Hello, Crotine!";
}

Crotine::Task<std::string> runTasks()
{
    auto intTask = produceValue();
    auto strTask = produceStrValue();

    intTask.execute_async();
    strTask.execute_async();

    auto data_1 = co_await strTask;
    std::cout << "String Task completed with value: " << data_1 << "\n";
    auto data_2 = std::to_string(co_await intTask);
    std::cout << "Integer Task completed with value: " << data_2 << "\n";

    co_return data_2 + " and " + data_1;
}

int main()
{
    std::cout << "Started Task.\n";
    auto tsk = runTasks();
    tsk.execute_async();
    std::cout << tsk.getPromise().getWaitedValue() << "\n";
    std::cout << "All tasks completed successfully.\n";
    return 0;
}