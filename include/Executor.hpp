#pragma once
#include <thread>
#include <functional>

namespace Crotine
{
    class Executor
    {
        public:
            virtual ~Executor() = default;
            virtual void execute(std::function<void()> func)
            {
                std::thread(std::move(func)).detach();
            }
        public:
            static Executor& getDefaultExecutor()
            {
                static Executor defaultExecutor;
                return defaultExecutor;
            }
    };
}