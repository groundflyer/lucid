// -*- C++ -*-
// seq.hpp
//

#pragma once

#include <type_traits>

namespace yapt
{
    namespace detail
    {
        template <typename BinaryOp, typename Operand>
        struct fold_wrapper
        {
            using ResultType = typename std::result_of_t<BinaryOp(const Operand&, const Operand&)>;

            BinaryOp op;
            ResultType operand;

            constexpr
            fold_wrapper(BinaryOp _op,
                         const Operand& _operand) : op(_op), operand(_operand) {}

            constexpr
            fold_wrapper() = delete;

            constexpr
            fold_wrapper(const fold_wrapper&) = delete;

            template <typename OperandB>
            constexpr decltype(auto)
            operator%(const fold_wrapper<BinaryOp, OperandB>& rhs) const noexcept
            { return fold_wrapper(op, op(operand, rhs.operand)); }
        };
    }

    template <typename BinaryOp, typename Init, typename ... Args>
    constexpr decltype(auto)
    reduce(BinaryOp&& op, Init&& init, Args&&... args) noexcept
    { return (detail::fold_wrapper(op, init) % ... % detail::fold_wrapper(op, args)).operand; }
}
