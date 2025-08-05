#pragma once
#include <queue>
#include "Executor.hpp"
#include "AutoThread.hpp"

namespace Crotine
{  
    class Xecutor : public Executor
    {
        private:
            std::deque<AutoThread> threads;
        private:
            std::mutex mtx;
        private:
            std::condition_variable notifier;
        private:
            std::chrono::milliseconds timeout;
        public:
            void execute(std::function<void()> func) override;
        public:
            Xecutor(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
            ~Xecutor() = default;
    };

    Xecutor::Xecutor(std::chrono::milliseconds timeout) : timeout(timeout)  {}

    void Xecutor::execute(std::function<void()> func)
    {
        std::lock_guard<std::mutex> lock_(mtx);
        
        // if there is no thread in the pool, create one
        if(threads.empty())
        {
            threads.emplace_back(notifier , timeout);
            threads.back().setExpireCallback([this]()
            {
                // if the thread is expired, remove it from the pool
                // only the last thread in the pool can expire
                std::lock_guard<std::mutex> lock(mtx);
                threads.pop_back();
            });
        }

        // get the first thread in the pool
        AutoThread thread = threads.front();
        threads.pop_front();

        // set the given task along with a callback 
        // to put the thread back to the pool
        thread.setTask(std::move([func = std::move(func) , this , thread = std::move(thread)]() mutable
        {
            func();
            // the most recently used thread will be put to the front of the pool
            // so that other threads can die if they are not used for a long time
            std::lock_guard<std::mutex> lock(mtx);
            threads.emplace_front(std::move(thread));
        }));

        notifier.notify_one();
    }
}