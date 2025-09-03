#include <queue>
#include <mutex>
#include <condition_variable>

namespace Crotine
{
    template<typename T>
    class BlockChannel
    {
        private:
            bool _closed = false;
        private:
           std::queue<T> _queue;
           std::mutex _mutex;
           std::condition_variable _notifier;
        public:
            BlockChannel() = default;
            ~BlockChannel() = default;
        public:
            void put(const T& item)
            {
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _queue.push(item);
                }
                _notifier.notify_one();
            }
            std::optional<T> take()
            {
                std::unique_lock<std::mutex> _lock(_mutex);
                _notifier.wait(_lock, [this] { return !_queue.empty() or _closed; });
                if(!_closed)
                {
                    auto item = std::move(_queue.front());
                    _queue.pop();
                    return item;
                }
                return std::nullopt;
            }
            std::optional<T> try_take_for(const std::chrono::milliseconds& timeout)
            {
                std::unique_lock<std::mutex> _lock(_mutex);
                if(_notifier.wait_for(_lock, timeout, [this] { return !_queue.empty() or _closed; }) && !_closed)
                {
                    auto item = std::move(_queue.front());
                    _queue.pop();
                    return item;
                }
                return std::nullopt;
            }
            void close()
            {
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _closed = true;
                }
                _notifier.notify_all();
            }
    };
}