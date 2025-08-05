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
}