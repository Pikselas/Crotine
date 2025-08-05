#include <string>
#include <iostream>

#include "../include/Task.hpp"
#include "../include/Xecutor.hpp"
#include "../include/utils/Context.hpp"

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
    auto squareTask = computeSquare(3);
    auto cubeTask = computeCube(3);

    std::cout << "Tasks started, waiting for results...\n";
    auto& exec_ctx = co_await Crotine::get_Execution_Context{};
    std::cout << "Execution ctx retrieved for merging results.\n";

    squareTask.set_execution_ctx(exec_ctx);
    cubeTask.set_execution_ctx(exec_ctx);

    squareTask.execute_async();
    cubeTask.execute_async();

    auto squareResult = co_await squareTask;
    std::cout << "Square result received: " << squareResult << "\n";
    auto cubeResult = co_await cubeTask;
    std::cout << "Cube result received: " << cubeResult << "\n";

    std::cout << "Results received: Square = " << squareResult << ", Cube = " << cubeResult << "\n";

    co_return "Results: " + std::to_string(squareResult) + " and " + std::to_string(cubeResult);
}

int main()
{
    auto pool = Crotine::Xecutor{};
    auto task = mergeResults();
    task.set_execution_ctx(pool);
    task.execute_async();
    std::cout << "Waiting for task to complete...\n";
    std::cout << task.getPromise().getWaitedValue() << "\n";
    std::cout << "All tasks completed successfully.\n";
    return 0;
}