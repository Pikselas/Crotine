#pragma once
#include "Executor.hpp"

namespace Crotine
{
    class PromiseBase
    {
        private:
            std::reference_wrapper<Executor> _execution_context;
        public:
            void set_execution_ctx(Executor& ctx)
            {
                _execution_context = std::ref(ctx);
            }
            Executor& get_execution_ctx()
            {
                return _execution_context.get();
            }
        public:
            PromiseBase() : _execution_context(Executor::getDefaultExecutor()) {}
            virtual ~PromiseBase() = default;
    };

    template<typename T>
    concept CPromiseBase = std::is_base_of_v<PromiseBase, T>;

    template<CPromiseBase T>
    class get_Execution_Context
    {
        private:
            std::optional<std::reference_wrapper<Executor>> _execution_context;
        public:
            bool await_ready() const noexcept
            {
                return _execution_context.has_value();
            }
            void await_suspend(std::coroutine_handle<T> handle)
            {
                _execution_context = static_cast<PromiseBase&>(handle.promise()).get_execution_ctx();
                std::cout << "Execution pool retrieved for calling coroutine\n";
                handle.resume();
            }
            auto await_resume() -> Executor&
            {
                if (!_execution_context.has_value())
                {
                    throw std::runtime_error("Execution pool not set");
                }
                std::cout << "Execution pool resumed for calling coroutine and returned the pool\n";
                return _execution_context->get();
            }
    };
}