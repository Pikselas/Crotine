#pragma once
#include "Task.hpp"
#include "utils/Function.hpp"

namespace Crotine
{
    class TaskRunner
    {
        private:
            Executor& executor;
        public:
            TaskRunner(Executor& exec) : executor(exec) {}
            TaskRunner() : executor(Executor::getDefaultExecutor()) {}
            ~TaskRunner() = default;
        public:
            template<typename F, typename... Args>
            requires CoroutineFunctionT<F, Args...> || NonCoroutineFunctionT<F, Args...>
            auto Run(F&& f , Args&&... args)
            {
                return RunTask(executor , std::forward<F>(f), std::forward<Args>(args)...);
            }
    };
}