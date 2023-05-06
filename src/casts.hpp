#pragma once

#include <memory.h>
#include "TID.hpp"

bool CanCastLossless(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to);
bool CanCast(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to);

std::shared_ptr<TIDValue> CastValue(const std::shared_ptr<TIDValue> & val, std::shared_ptr<TIDVariableType> type);
PrimitiveVariableType LeastCommonType(PrimitiveVariableType lhs, PrimitiveVariableType rhs);

PrimitiveVariableType NumericTypeFromString(const std::wstring & val);