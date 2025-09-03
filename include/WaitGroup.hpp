#include <mutex>
#include <atomic>
#include <condition_variable>

namespace Crotine
{
    class WaitGroup
    {
        private:
            std::atomic_uint _count = 0;
            std::mutex _mtx;
            std::condition_variable _cv;
        public:
            WaitGroup() = default;
            ~WaitGroup() = default;
        public:
            void add(int delta)
            {
                _count.fetch_add(delta);
            }
            void done()
            {
                if(_count.fetch_sub(1) == 1)
                {
                    _cv.notify_all();
                }
            }
            void wait()
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _cv.wait(lock, [this]() { return _count.load() == 0; });
            }
            unsigned int count() const
            {
                return _count.load();
            }
    };
}