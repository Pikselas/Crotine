#pragma once
#include <concepts>
#include <type_traits>

#include "../Task.hpp"

namespace Crotine
{
    template<typename T>
    struct TaskType
    {
        using type = T;
    };

    template<typename T>
    struct TaskType<Task<T>>
    {
        using type = T;
    };

    // template<typename Function>
    // concept CoroutineFunctionT = std::is_invocable_v<Function> && std::is_same_v<Task<typename TaskType<std::invoke_result_t<Function>>::type> , std::invoke_result_t<Function>>;

    template<typename Function , typename...Args>
    concept CoroutineFunctionT = std::is_invocable_v<Function , Args...> && requires(Function f , Args... args)
    {
        {f(std::forward<Args>(args)...)} -> std::same_as<Task<typename TaskType<std::invoke_result_t<Function , Args...>>::type>>;
    };

    template<typename Function, typename... Args>
    concept NonCoroutineFunctionT = std::is_invocable_v<Function, Args...> && !CoroutineFunctionT<Function, Args...>;

    template<typename Function, typename... Args>
    requires CoroutineFunctionT<Function, Args...>
    inline auto CreateTask(Function&& func , Args&&... args) -> std::invoke_result_t<Function, Args...>
    {
        return std::invoke(std::forward<Function>(func), std::forward<Args>(args)...);
    }

    template<typename Function, typename... Args>
    requires NonCoroutineFunctionT<Function, Args...>
    inline auto CreateTask(Function&& func, Args... args) -> Task<std::invoke_result_t<Function, Args...>>
    {
        co_return std::invoke(std::forward<Function>(func), std::forward<Args>(args)...);
    }

    template<typename Function, typename... Args>
    requires CoroutineFunctionT<Function, Args...> || NonCoroutineFunctionT<Function, Args...>
    inline auto RunTask(Function&& func, Args&&... args)
    {
        auto task = CreateTask(std::forward<Function>(func), std::forward<Args>(args)...);
        task.execute_async();
        return task;
    }

    template<typename Function, typename... Args>
    requires CoroutineFunctionT<Function, Args...> || NonCoroutineFunctionT<Function,Args...>
    inline auto RunTask(Executor& ctx, Function&& func, Args&&... args)
    {
        auto task = CreateTask(std::forward<Function>(func), std::forward<Args>(args)...);
        task.set_execution_ctx(ctx);
        task.execute_async();
        return task;
    }
}