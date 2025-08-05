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
                    auto final_suspend() noexcept -> std::suspend_always;
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
                    auto await_resume() -> const T&;
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
}

template <typename T>
Crotine::Task<T> Crotine::Task<T>::Promise::get_return_object()
{
    return Task<T>{Handle::from_promise(*this)};
}

template <typename T>
std::suspend_always Crotine::Task<T>::Promise::initial_suspend()
{
    return {};
}

template <typename T>
std::suspend_always Crotine::Task<T>::Promise::final_suspend() noexcept
{
    return {};
}

template <typename T>
void Crotine::Task<T>::Promise::return_value(const T& value)
{
    _value = value;
    _promise.set_value(value);
    for (auto& continuation : _continuations)
    {
        continuation(value);
    }
}

template <typename T>
void Crotine::Task<T>::Promise::unhandled_exception()
{
    std::terminate();
}

template <typename T>
Crotine::Task<T>::Promise::Promise() : _future(_promise.get_future()) {}

template <typename T>
bool Crotine::Task<T>::Promise::isResolved() const noexcept
{
    return _value.has_value();
}

template <typename T>
const T& Crotine::Task<T>::Promise::getValue() const
{
    if (!_value.has_value())
    {
        throw std::runtime_error("Value not set");
    }
    return _value.value();
}

template <typename T>
T Crotine::Task<T>::Promise::getWaitedValue()
{
    return _future.get();
}

template <typename T>
void Crotine::Task<T>::Promise::chainOnResolved(std::function<void()> continuation)
{
    _continuations.emplace_front([continuation](const T&){ continuation(); });
}

template <typename T>
void Crotine::Task<T>::Promise::chainOnResolved(std::function<void(const T&)> continuation)
{
    _continuations.emplace_front(std::move(continuation));
}

template <typename T>
Crotine::Task<T>::Task(Handle handle) : _handle(handle) {}

template <typename T>
Crotine::Task<T>::~Task()
{
    if (_handle)
    {
        _handle.destroy();
    }
}

template <typename T>
Crotine::Task<T>::Promise& Crotine::Task<T>::getPromise()
{
    return _handle.promise();
}

template <typename T>
void Crotine::Task<T>::execute_async()
{
    if (_handle)
    {
        getPromise().get_execution_ctx().execute([this]()
        {
            _handle.resume();
        });
    }
}

template <typename T>
void Crotine::Task<T>::set_execution_ctx(Executor& ctx)
{
    getPromise().set_execution_ctx(ctx);
}

template <typename T>
Crotine::Task<T>::Awaiter::Awaiter(Promise& promise) : _promise(promise)
{}

template <typename T>
bool Crotine::Task<T>::Awaiter::await_ready() const noexcept
{
    return _promise.isResolved();
}

template <typename T>
void Crotine::Task<T>::Awaiter::await_suspend(std::coroutine_handle<> handle) const
{
    if (!_promise.isResolved())
    {
        _promise.chainOnResolved([handle]() 
        {
            auto typed_handle = std::coroutine_handle<PromiseBase>::from_address(handle.address());
            auto& base = static_cast<PromiseBase&>(typed_handle.promise());
            base.get_execution_ctx().execute([handle]()
            {
                handle.resume();
            });
        });
    }
}

template <typename T>
const T& Crotine::Task<T>::Awaiter::await_resume()
{
    return _promise.getValue();
}

template <typename T>
Crotine::Task<T>::Awaiter Crotine::Task<T>::operator co_await()
{
    return { getPromise() };
}