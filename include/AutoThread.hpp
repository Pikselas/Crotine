#pragma once
#include <thread>
#include <memory>
#include <chrono>
#include <functional>
#include <condition_variable>

#include "WaitGroup.hpp"
#include "BlockChannel.hpp"

namespace Crotine
{
    class AutoThread
    {
        public:
            struct thread_context
            {
                BlockChannel<std::function<void()>>& tasks;
                std::chrono::milliseconds timeout;
                std::function<void()> expire_callback;
                public:
                    thread_context(BlockChannel<std::function<void()>>& task_channel , std::chrono::milliseconds timeout , std::function<void()> expire_callback)
                        : tasks(task_channel) , timeout(timeout) , expire_callback(expire_callback) {}
            };
        public:
            AutoThread(thread_context context);
            ~AutoThread() = default;
    };

    AutoThread::AutoThread(thread_context context)
    {
        std::thread([context]() mutable
        {
            while(true)
            {
                if(auto task = context.tasks.try_take_for(context.timeout); task)
                {
                    (*task)();
                    // going out of scope will destroy the task
                    // task destruction is necessary for some tasks
                }
                else
                    break;
            }
            if(context.expire_callback)
            {
                context.expire_callback();
            }
        }).detach();
    }
}