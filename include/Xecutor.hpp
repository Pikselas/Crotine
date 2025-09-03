#pragma once
#include <queue>
#include "Executor.hpp"
#include "AutoThread.hpp"

namespace Crotine
{  
    class Xecutor : public Executor
    {
        private:
            std::atomic_uint _idle_threads = 0;
        private:
            std::chrono::milliseconds _timeout;
        private:
            WaitGroup _wait_group;
        private:
            BlockChannel<std::function<void()>> _tasks;
        private:
            unsigned int _max_worker = 0;
        public:
            void execute(std::function<void()> func) override;
        public:
            Xecutor(unsigned int max_worker = std::thread::hardware_concurrency() , std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
            ~Xecutor();
    };

    Xecutor::Xecutor(unsigned int max_worker, std::chrono::milliseconds timeout) : _max_worker(max_worker) , _timeout(timeout) {}

    Xecutor::~Xecutor()
    {
        _tasks.close();
        _wait_group.wait();
    }

    void Xecutor::execute(std::function<void()> func)
    {
        // if there is no active thread, create one
        if((_idle_threads.load() == 0) && (_wait_group.count() < _max_worker))
        {
            _idle_threads.fetch_add(1);
            _wait_group.add(1);
            AutoThread(AutoThread::thread_context{_tasks , _timeout , [this]()
            {
                _idle_threads.fetch_sub(1);
                _wait_group.done();
            }});
        }

        _tasks.put([_task = func , this]()
        {
            _idle_threads.fetch_sub(1);
            if(_task)
                _task();
            _idle_threads.fetch_add(1);
        });
    }
}