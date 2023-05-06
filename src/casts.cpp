#include "casts.hpp"
#include "TID.hpp"
#include "exceptions.hpp"

#include <queue>

bool set_up_casting = false;

uint8_t layer[kPrimitiveVariableTypeCount];

// bool -> int8 -> uint8 -> int16 -> uint16 -> int32 -> uint32 -> int64 -> uint64 -> f32 -> f64
//          ^
//          |
//         char

void SetUpCastingPrimitives() {
#define SetLayer(type, val) layer[static_cast<uint8_t>(PrimitiveVariableType::type)] = val
  SetLayer(kBool, 0);
  SetLayer(kChar, 0);
  SetLayer(kInt8, 1);
  SetLayer(kUint8, 2);
  SetLayer(kInt16, 3);
  SetLayer(kUint16, 4);
  SetLayer(kInt32, 5);
  SetLayer(kUint32, 6);
  SetLayer(kInt64, 7);
  SetLayer(kUint64, 8);
  SetLayer(kF32, 9);
  SetLayer(kF64, 10);
#undef SetLayer
  set_up_casting = true;
}

int8_t CastValue(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  if (from == to) return 1;
  if (from == nullptr || to == nullptr) return -1;
//  if ((from->IsConst() && !to->IsConst()) || (!from->IsReference() && to->IsReference()))
//    return -1;
  if (from->GetType() != to->GetType()) return -1;
  if (from->GetType() != VariableType::kPrimitive) {
    // we don't cast complex structs and functions, also arrays, but can if we really want
    // however pointers - we do convert these, type punning go weeeeeeeeee
    return from->GetType() == VariableType::kPointer ? 1 : -1;
  }
  // which primitives we can cast to which
  if (!set_up_casting) SetUpCastingPrimitives();
  PrimitiveVariableType from_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(from)->GetPrimitiveType();
  PrimitiveVariableType to_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(to)->GetPrimitiveType();
  //return can_cast[static_cast<uint8_t>(from_pr)][static_cast<uint8_t>(to_pr)];
  return GetSizeOfPrimitive(from_pr) <= GetSizeOfPrimitive(to_pr);
}

bool CanCastLossless(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValue(from, to) == 1;
}

bool CanCast(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValue(from, to) != -1;
}

std::shared_ptr<TIDValue> CastValue(const std::shared_ptr<TIDValue> & val, std::shared_ptr<TIDVariableType> type) {
  if (!type || !val->GetType())
    return std::make_shared<TIDTemporaryValue>(nullptr);
  type = SetParamsToType(type, val->GetType()->IsConst(), val->GetType()->IsReference());
  if (!CanCast(val->GetType(), type))
    throw UnableToCast(val, type);
  if (val->GetType() == type) return val;
  else return std::make_shared<TIDTemporaryValue>(SetParamsToType(type, true, false));
}

PrimitiveVariableType LeastCommonType(PrimitiveVariableType lhs, PrimitiveVariableType rhs) {
  if (lhs == rhs) return lhs;
  if (lhs == PrimitiveVariableType::kBool && rhs == PrimitiveVariableType::kChar ||
      lhs == PrimitiveVariableType::kChar && rhs == PrimitiveVariableType::kBool)
    return PrimitiveVariableType::kInt8;
  uint8_t ulhs = static_cast<uint8_t>(lhs);
  uint8_t urhs = static_cast<uint8_t>(rhs);
  if (layer[ulhs] < layer[urhs]) return rhs;
  else return lhs;
}

PrimitiveVariableType NumericTypeFromString(const std::wstring & val) {
  bool is_decimal = false;
  for (wchar_t ch : val)
    if (ch == L'.')
      is_decimal = true;
  std::wstring suf;
  for (size_t i = val.size() - 1; i > 0 && !std::isdigit(val[i]); --i) {
    suf.push_back(val[i]);
  }
  if (is_decimal)
    return suf.empty() ? PrimitiveVariableType::kF32 : PrimitiveVariableType::kF64;
  else {
    if (suf.empty()) return PrimitiveVariableType::kInt32;
    bool us = false;
    if (suf.back() == 'u' || suf.back() == 'U') {
      us = true;
      suf.pop_back();
    }
    if (suf.empty()) return PrimitiveVariableType::kUint32;
    wchar_t ch = suf.back();
    switch (ch) {
      case 't':
      case 'T':
        return us ? PrimitiveVariableType::kUint8 : PrimitiveVariableType::kInt8;
      case 's':
      case 'S':
        return us ? PrimitiveVariableType::kUint16 : PrimitiveVariableType::kInt16;
      case 'i':
      case 'I':
        return us ? PrimitiveVariableType::kUint32 : PrimitiveVariableType::kInt32;
      case 'l':
      case 'L':
        return us ? PrimitiveVariableType::kUint64 : PrimitiveVariableType::kInt64;
    }
  }
  // should be unreachable
  return PrimitiveVariableType::kInt32;
}