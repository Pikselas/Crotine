#pragma once
#include <thread>
#include <future>
#include <optional>
#include <functional>
#include <coroutine>
#include <forward_list>

#include "PromiseBase.hpp"

namespace Crotine
{
    template <typename T>
    class Task 
    {
        public:
            class Promise : public PromiseBase
            {
                private:
                    std::optional<T> _value;
                    std::promise<T> _promise;
                    std::future<T> _future;
                private:
                    std::forward_list<std::function<void(const T&)>> _continuations;
                public:
                    auto get_return_object() -> Task<T>;
                    auto initial_suspend() -> std::suspend_always;
                    auto final_suspend() noexcept -> std::suspend_never;
                    void return_value(const T& value);
                    void unhandled_exception();
                public:
                    Promise();
                    ~Promise() = default;
                public:
                    bool isResolved() const noexcept;
                    auto getValue() const -> const T&;
                    auto getWaitedValue() -> T;
                public:
                    void chainOnResolved(std::function<void()> continuation);
                    void chainOnResolved(std::function<void(const T&)> continuation);
            };
            class Awaiter
            {
                private:
                    Promise& _promise;
                public:
                    Awaiter(Promise& promise);
                public:
                    bool await_ready() const noexcept;
                    void await_suspend(std::coroutine_handle<> handle) const;
                    auto await_resume();
            };
        using promise_type = Promise;
        using Handle = std::coroutine_handle<Promise>;
        private:
            Handle _handle;
        public:
            Task(Handle handle);
            Task(const Task&) = delete;
            Task(Task&& other) noexcept = default;
            ~Task();
        public:
            Task& operator=(const Task&) = delete;
            Task& operator=(Task&& other) noexcept = default;
        public:
            void execute_async();
            void set_execution_ctx(Executor& ctx);
        public:
            auto getPromise() -> Promise&;
        public:
            auto operator co_await() -> Awaiter;
    };

    template <>
    class Task<void>::Promise : public PromiseBase
    {
        private:
            std::promise<void> _promise;
            std::future<void> _future;
            std::forward_list<std::function<void()>> _continuations;
        public:
            auto get_return_object() -> Task<void>;
            auto initial_suspend() -> std::suspend_always;
            auto final_suspend() noexcept -> std::suspend_never;
            void return_void();
            void unhandled_exception();
        public:
            Promise();
            ~Promise() = default;
        public:
            bool isResolved() const noexcept;
            void Wait();
        public:
            void chainOnResolved(std::function<void()> continuation);
    };
}

inline Crotine::Task<void> Crotine::Task<void>::Promise::get_return_object()
{
    return Task<void>{Handle::from_promise(*this)};
}

inline std::suspend_always Crotine::Task<void>::Promise::initial_suspend()
{
    return {};
}

inline std::suspend_never Crotine::Task<void>::Promise::final_suspend() noexcept
{
    return {};
}

inline bool Crotine::Task<void>::Promise::isResolved() const noexcept
{
    return _future.valid() && _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

inline void Crotine::Task<void>::Promise::Wait()
{
    _future.get();
}

inline void Crotine::Task<void>::Promise::return_void()
{
    _promise.set_value();
    for (auto& continuation : _continuations)
    {
        continuation();
    }
}

inline void Crotine::Task<void>::Promise::unhandled_exception()
{
    std::terminate();
}

inline Crotine::Task<void>::Promise::Promise() : _future(_promise.get_future()) {}

inline void Crotine::Task<void>::Promise::chainOnResolved(std::function<void()> continuation)
{
    _continuations.emplace_front(std::move(continuation));
}

template <typename T>
inline Crotine::Task<T> Crotine::Task<T>::Promise::get_return_object()
{
    return Task<T>{Handle::from_promise(*this)};
}

template <typename T>
inline std::suspend_always Crotine::Task<T>::Promise::initial_suspend()
{
    return {};
}

template <typename T>
inline std::suspend_never Crotine::Task<T>::Promise::final_suspend() noexcept
{
    return {};
}

template <typename T>
inline void Crotine::Task<T>::Promise::return_value(const T& value)
{
    _value = value;
    _promise.set_value(value);
    for (auto& continuation : _continuations)
    {
        continuation(value);
    }
}

template <typename T>
inline void Crotine::Task<T>::Promise::unhandled_exception()
{
    std::terminate();
}

template <typename T>
inline Crotine::Task<T>::Promise::Promise() : _future(_promise.get_future()) {}

template <typename T>
inline bool Crotine::Task<T>::Promise::isResolved() const noexcept
{
    return _value.has_value();
}

template <typename T>
inline const T& Crotine::Task<T>::Promise::getValue() const
{
    if (!_value.has_value())
    {
        throw std::runtime_error("Value not set");
    }
    return _value.value();
}

template <typename T>
inline T Crotine::Task<T>::Promise::getWaitedValue()
{
    return _future.get();
}

template <typename T>
inline void Crotine::Task<T>::Promise::chainOnResolved(std::function<void()> continuation)
{
    _continuations.emplace_front([continuation](const T&){ continuation(); });
}

template <typename T>
inline void Crotine::Task<T>::Promise::chainOnResolved(std::function<void(const T&)> continuation)
{
    _continuations.emplace_front(std::move(continuation));
}

template <typename T>
inline Crotine::Task<T>::Task(Handle handle) : _handle(handle) {}

template <typename T>
inline Crotine::Task<T>::~Task()
{
    // if (_handle)
    // {
    //     _handle.destroy();
    // }
}

template <typename T>
inline Crotine::Task<T>::Promise& Crotine::Task<T>::getPromise()
{
    return _handle.promise();
}

template <typename T>
inline void Crotine::Task<T>::execute_async()
{
    if (_handle)
    {
        getPromise().get_execution_ctx().execute([handle = _handle]()
        {
            handle.resume();
        });
    }
}

template <typename T>
inline void Crotine::Task<T>::set_execution_ctx(Executor& ctx)
{
    getPromise().set_execution_ctx(ctx);
}

template <typename T>
inline Crotine::Task<T>::Awaiter::Awaiter(Promise& promise) : _promise(promise)
{}

template <typename T>
inline bool Crotine::Task<T>::Awaiter::await_ready() const noexcept
{
    return _promise.isResolved();
}

template <typename T>
inline void Crotine::Task<T>::Awaiter::await_suspend(std::coroutine_handle<> handle) const
{
    if (!_promise.isResolved())
    {
        _promise.chainOnResolved([handle]() 
        {
            auto typed_handle = std::coroutine_handle<PromiseBase>::from_address(handle.address());
            typed_handle.promise().get_execution_ctx().execute([handle]()
            {
                handle.resume();
            });
        });
    }
}

template <typename T>
inline auto Crotine::Task<T>::Awaiter::await_resume()
{
    if constexpr(!std::is_void_v<T>)
        return _promise.getValue();
}

template <typename T>
inline Crotine::Task<T>::Awaiter Crotine::Task<T>::operator co_await()
{
    return { getPromise() };
}