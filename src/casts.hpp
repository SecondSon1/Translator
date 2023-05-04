#pragma once

#include <memory.h>
#include "TID.hpp"

bool CanCastLossless(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to);
bool CanCast(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to);

std::shared_ptr<TIDVariableType> LeastCommonType(std::shared_ptr<TIDVariableType> lhs,
                                                 std::shared_ptr<TIDVariableType> rhs);
