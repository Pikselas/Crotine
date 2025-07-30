#include <iostream>
#include "crotine.hpp"

Crotine::Task<int> produceValue()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    co_return 42;
}

Crotine::Task<std::string> produceStrValue()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    co_return "Hello, Crotine!";
}

Crotine::Task<int> runTasks()
{
    auto intTask = produceValue();
    auto strTask = produceStrValue();

    std::cout << "Tasks started...\n";

    intTask.execute_async();
    strTask.execute_async();

    int value = co_await intTask;
    std::string strValue = co_await strTask;
    
    std::cout << "Produced int: " << value << "\n";
    std::cout << "Produced string: " << strValue << "\n";

    co_return 0;
}

int main()
{
    std::cout << runTasks().getPromise().getWaitedValue() << "\n";
    std::cout << "All tasks completed successfully.\n";
    return 0;
}