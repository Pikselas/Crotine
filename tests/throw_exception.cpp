#include "../include/Task.hpp"
#include "../include/utils/Function.hpp"

Crotine::Task<void> Use_op(Crotine::Task<int> task)
{
    try
    {
        int result = co_await task;
        std::cout << "Result: " << result << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught: " << e.what() << "\n";
        throw std::runtime_error("Error in Use_op task");
    }
}

int main()
{
    auto Coro_divide = [](int param) -> Crotine::Task<int>
    {
        if(param == 0)
        {
            throw std::runtime_error("Division by zero in coroutine task");
        }
        co_return param / 42;
    };
    
    try
    {
        Crotine::RunTask(Use_op, Crotine::RunTask(Coro_divide, 0)).getPromise().Wait();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << "\n";
    }
    return 0;
}