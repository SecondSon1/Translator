#pragma once

#include <memory.h>
#include "operators.hpp"
#include "TID.hpp"

std::shared_ptr<TIDVariableType> UnaryPrefixOperation(UnaryPrefixOperator op, const std::shared_ptr<TIDVariableType> & type, const Lexeme & lexeme);
std::shared_ptr<TIDVariableType> UnaryPostfixOperation(const std::shared_ptr<TIDVariableType> & type, UnaryPostfixOperator op, const Lexeme & lexeme);
std::shared_ptr<TIDVariableType> BinaryOperation(const std::shared_ptr<TIDVariableType> & lhs, BinaryOperator op, const std::shared_ptr<TIDVariableType> & rhs, const Lexeme & lexeme);
