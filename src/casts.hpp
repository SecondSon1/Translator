#pragma once

#include <memory.h>
#include "TID.hpp"
#include "generation.hpp"

bool CanCastLossless(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to);
bool CanCast(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to);

std::shared_ptr<TIDValue> CastValue(const std::shared_ptr<TIDValue> & val, std::shared_ptr<TIDVariableType> type);
PrimitiveVariableType LeastCommonType(PrimitiveVariableType lhs, PrimitiveVariableType rhs);

PrimitiveVariableType NumericTypeFromString(const std::wstring & val);
uint64_t IntegerFromString(const std::wstring & val, PrimitiveVariableType type);
uint64_t DecimalFromString(const std::wstring & val, PrimitiveVariableType type);
void Cast(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to, RPN & rpn);
