#pragma once

#include <memory.h>
#include "operators.hpp"
#include "TID.hpp"
#include "generation.hpp"

std::shared_ptr<TIDValue> UnaryPrefixOperation(UnaryPrefixOperator op,
    const std::shared_ptr<TIDValue> & type, const Lexeme & lexeme);
std::shared_ptr<TIDValue> UnaryPostfixOperation(const std::shared_ptr<TIDValue> & type,
    UnaryPostfixOperator op, const Lexeme & lexeme);
//std::shared_ptr<TIDValue> BinaryOperation(const std::shared_ptr<TIDValue> & lhs, BinaryOperator op,
//    const std::shared_ptr<TIDValue> & rhs, const Lexeme & lexeme);

std::shared_ptr<TIDValue> UnaryPrefixOperationRPN(UnaryPrefixOperator op,
    const std::shared_ptr<TIDValue> & type, const Lexeme & lexeme, RPN & rpn);
std::shared_ptr<TIDValue> UnaryPostfixOperationRPN(const std::shared_ptr<TIDValue> & type, 
    UnaryPostfixOperator op, const Lexeme & lexeme, RPN & rpn);
std::shared_ptr<TIDValue> BinaryOperationRPN(const std::shared_ptr<TIDValue> & lhs, BinaryOperator op,
    const std::shared_ptr<TIDValue> & rhs, const Lexeme & lexeme, RPN & rpn);
