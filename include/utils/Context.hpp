#pragma once

namespace Crotine
{
    class get_Execution_Context
    {
        private:
            std::optional<std::reference_wrapper<Executor>> _execution_context;
        public:
            bool await_ready() const noexcept
            {
                return _execution_context.has_value();
            }
            void await_suspend(std::coroutine_handle<> handle)
            {
                 auto typed_handle = std::coroutine_handle<PromiseBase>::from_address(handle.address());
                _execution_context = typed_handle.promise().get_execution_ctx();
                _execution_context->get().execute([handle]()
                {
                    handle.resume();
                });
            }
            auto await_resume() -> Executor&
            {
                if (!_execution_context.has_value())
                {
                    throw std::runtime_error("Execution pool not set");
                }
                return _execution_context->get();
            }
    };
}