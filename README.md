## `Crotine` 
### C++ coroutine library for asyncronous operations
> C++20 experimental framework for my own learning purpose `not production ready`
* Coroutine `Task`
* `Execution` Context
* `Xecutor` thread pools
* Utility classes
    * `get_Execution_Context` for retrieving execution contexts
### Examples
```C++
#include <string>
#include <iostream>

#include "Task.hpp"
#include "Xecutor.hpp"
#include "utils/Context.hpp"

Crotine::Task<int> computeSquare(int num)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    co_return num * num;
}

Crotine::Task<int> computeCube(int num)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    co_return num * num * num;
}

Crotine::Task<std::string> mergeResults()
{

    // creating two child coroutines
    auto squareTask = computeSquare(3);
    auto cubeTask = computeCube(3);

    // getting current execution environment
    auto& exec_ctx = co_await Crotine::get_Execution_Context{};

    // childs are going to be executed on the same execution environment
    squareTask.set_execution_ctx(exec_ctx);
    cubeTask.set_execution_ctx(exec_ctx);

    // started execution of the childs
    squareTask.execute_async();
    cubeTask.execute_async();

    // resume when task is completed
    auto squareResult = co_await squareTask;
    auto cubeResult = co_await cubeTask;

    // returning from the task [marking as the tasks are completed]
    co_return "Results: " + std::to_string(squareResult) + " and " + std::to_string(cubeResult);
}

int main()
{
    // creating an execution context
    // and executing the coroutine in that context
    auto pool = Crotine::Xecutor{};
    auto task = mergeResults();
    task.set_execution_ctx(pool);

    // coroutine starts execution
    task.execute_async();

    std::cout << "Waiting for task to complete...\n";

    // main thread is blocked till the Task is completed
    std::cout << task.getPromise().getWaitedValue() << "\n";
    std::cout << "All tasks completed successfully.\n";
    return 0;
}
```
