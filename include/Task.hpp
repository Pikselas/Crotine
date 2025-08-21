#pragma once
#include <thread>
#include <future>
#include <variant>
#include <optional>
#include <functional>
#include <coroutine>
#include <forward_list>

#include <iostream>

#include "PromiseBase.hpp"

namespace Crotine
{
    class Final_suspension_awaiter
    {
        private:
            std::variant<std::suspend_always, std::suspend_never> _suspension;
        public:
            bool await_ready() const noexcept;
            void await_resume() const noexcept;
            void await_suspend(std::coroutine_handle<> handle) const noexcept;
        public:
            Final_suspension_awaiter(std::suspend_never);
            Final_suspension_awaiter(std::suspend_always);
    };

    template <typename T>
    class Task 
    {
        public:
            class Promise : public PromiseBase
            {
                private:
                    Final_suspension_awaiter _suspension_awaiter;
                private:
                    std::optional<T> _value;
                    std::promise<T> _promise;
                    std::future<T> _future;
                private: 
                    std::forward_list<std::function<void(const T&)>> _continuations;
                    std::forward_list<std::function<void(std::exception_ptr)>> _exception_handlers;
                public:
                    auto get_return_object() -> Task<T>;
                    auto initial_suspend() -> std::suspend_always;
                    auto final_suspend() noexcept -> Final_suspension_awaiter;
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
                public:
                    void chainOnException(std::function<void()> handler);
                    void chainOnException(std::function<void(std::exception_ptr)> handler);
                public:
                    void setFinalSuspensionAwaiter(Final_suspension_awaiter awaiter);
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
            Task(Task&& other) noexcept;
            ~Task();
        public:
            Task& operator=(const Task&) = delete;
            Task& operator=(Task&& other) noexcept;
        public:
            void execute_async();
            void set_execution_ctx(Executor& ctx);
        public:
            auto getPromise() -> Promise&;
        public:
            auto operator co_await() -> Awaiter;
        public:
            void detach();
    };

    template <>
    class Task<void>::Promise : public PromiseBase
    {
         private:
            Final_suspension_awaiter _suspension_awaiter;
        private:
            std::promise<void> _promise;
            std::future<void> _future;
            std::forward_list<std::function<void()>> _continuations;
            std::forward_list<std::function<void(std::exception_ptr)>> _exception_handlers;
        public:
            auto get_return_object() -> Task<void>;
            auto initial_suspend() -> std::suspend_always;
            auto final_suspend() noexcept -> Final_suspension_awaiter;
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
        public:
            void chainOnException(std::function<void()> handler);
            void chainOnException(std::function<void(std::exception_ptr)> handler);
        public:
            void setFinalSuspensionAwaiter(Final_suspension_awaiter awaiter);
    };
}

inline bool Crotine::Final_suspension_awaiter::await_ready() const noexcept
{
    return std::visit([](auto&& suspension) { return suspension.await_ready(); }, _suspension);
}

inline void Crotine::Final_suspension_awaiter::await_resume() const noexcept
{
    std::visit([](auto&& suspension) { suspension.await_resume(); }, _suspension);
}

inline void Crotine::Final_suspension_awaiter::await_suspend(std::coroutine_handle<> handle) const noexcept
{
    std::visit([handle](auto&& suspension) { suspension.await_suspend(handle); }, _suspension);
}

inline Crotine::Final_suspension_awaiter::Final_suspension_awaiter(std::suspend_never _s) : _suspension(_s) 
{}

inline Crotine::Final_suspension_awaiter::Final_suspension_awaiter(std::suspend_always _s) : _suspension(_s) 
{}

inline Crotine::Task<void> Crotine::Task<void>::Promise::get_return_object()
{
    return Task<void>{Handle::from_promise(*this)};
}

inline std::suspend_always Crotine::Task<void>::Promise::initial_suspend()
{
    return {};
}

inline Crotine::Final_suspension_awaiter Crotine::Task<void>::Promise::final_suspend() noexcept
{
    return _suspension_awaiter;
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
    auto exception = std::current_exception();
    _promise.set_exception(exception);
    for (auto& handler : _exception_handlers)
    {
        handler(exception);
    }
}

inline Crotine::Task<void>::Promise::Promise() : _suspension_awaiter(std::suspend_always{}) , _future(_promise.get_future()) {}

inline void Crotine::Task<void>::Promise::chainOnResolved(std::function<void()> continuation)
{
    _continuations.emplace_front(std::move(continuation));
}

inline void Crotine::Task<void>::Promise::chainOnException(std::function<void()> handler)
{
    _exception_handlers.emplace_front([handler](std::exception_ptr){ handler(); });
}

inline void Crotine::Task<void>::Promise::chainOnException(std::function<void(std::exception_ptr)> handler)
{
    _exception_handlers.emplace_front(std::move(handler));
}

inline void Crotine::Task<void>::Promise::setFinalSuspensionAwaiter(Final_suspension_awaiter awaiter)
{
    _suspension_awaiter = std::move(awaiter);
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
inline Crotine::Final_suspension_awaiter Crotine::Task<T>::Promise::final_suspend() noexcept
{
    return _suspension_awaiter;
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
    auto exception = std::current_exception();
    _promise.set_exception(exception);
    for (auto& handler : _exception_handlers)
    {
        handler(exception);
    }
}

template <typename T>
inline Crotine::Task<T>::Promise::Promise() : _suspension_awaiter(std::suspend_always{}) , _future(_promise.get_future()) {}

template <typename T>
inline bool Crotine::Task<T>::Promise::isResolved() const noexcept
{
    return _future.valid() && _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
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
inline void Crotine::Task<T>::Promise::chainOnException(std::function<void()> handler)
{
    _exception_handlers.emplace_front([handler](std::exception_ptr){ handler(); });
}

template <typename T>
inline void Crotine::Task<T>::Promise::chainOnException(std::function<void(std::exception_ptr)> handler)
{
    _exception_handlers.emplace_front(std::move(handler));
}

template <typename T>
inline void Crotine::Task<T>::Promise::setFinalSuspensionAwaiter(Final_suspension_awaiter awaiter)
{
    _suspension_awaiter = std::move(awaiter);
}

template <typename T>
inline Crotine::Task<T>::Task(Handle handle) : _handle(handle) {}

template <typename T>
inline Crotine::Task<T>::Task(Task &&other) noexcept
{
    *this = std::move(other);
}

template <typename T>
inline Crotine::Task<T>::~Task()
{
    if (_handle)
    {
        _handle.destroy();
    }
}

template <typename T>
inline Crotine::Task<T>::Promise& Crotine::Task<T>::getPromise()
{
    return _handle.promise();
}

template <typename T>
inline Crotine::Task<T> &Crotine::Task<T>::operator=(Task &&other) noexcept
{
    _handle = std::exchange(other._handle, nullptr);
    return *this;
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
    auto resume_routine = [handle]() 
    {
        auto typed_handle = std::coroutine_handle<PromiseBase>::from_address(handle.address());
        typed_handle.promise().get_execution_ctx().execute([handle]()
        {
            handle.resume();
        });
    };
    if (!_promise.isResolved())
    {
        _promise.chainOnResolved(resume_routine);
        _promise.chainOnException(resume_routine);
    }
    else
    {
        resume_routine();
    }
}

template <typename T>
inline auto Crotine::Task<T>::Awaiter::await_resume()
{
    if constexpr(!std::is_void_v<T>)
        return _promise.getWaitedValue();
    else
        _promise.Wait();
}

template <typename T>
inline Crotine::Task<T>::Awaiter Crotine::Task<T>::operator co_await()
{
    return { getPromise() };
}

template <typename T>
inline void Crotine::Task<T>::detach()
{
    getPromise().setFinalSuspensionAwaiter(Final_suspension_awaiter{std::suspend_never{}});
    _handle = nullptr;
}