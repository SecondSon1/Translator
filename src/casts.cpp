#include "casts.hpp"
#include "TID.hpp"
#include "exceptions.hpp"
#include "generation.hpp"

#include <queue>
#include <cassert>

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

int8_t CastValueAsInt(std::shared_ptr<TIDValue> from, std::shared_ptr<TIDVariableType> to) {
  auto from_type = from->GetType();
  if (!from_type && !to) return 1;
  if (!from_type || !to) return -1;
  if (from == to) return 1;
  bool from_ref = from->GetValueType() == TIDValueType::kVariable || from_type->IsReference();
  bool to_ref = to->IsReference();
  if ((from_type->IsConst() && !to->IsConst()) || (!from_ref && to_ref))
    return -1;
  if (from_type->GetType() != to->GetType()) return -1;
  if (from_type->GetType() != VariableType::kPrimitive) {
    // we don't cast complex structs and functions, also arrays, but can if we really want
    // however pointers - we do convert these, type punning go weeeeeeeeee
    if (from_type->GetType() == VariableType::kPointer) return 1;
    else if (from_type->GetType() == VariableType::kArray) {
      auto val_type = std::dynamic_pointer_cast<TIDArrayVariableType>(from_type)->GetValue();
      auto to_type = std::dynamic_pointer_cast<TIDArrayVariableType>(to)->GetValue();
      return SetParamsToType(val_type, false, false) == SetParamsToType(to_type, false, false) ? 1 : -1;
    }
    return -1;
  }
  // which primitives we can cast to which
  if (!set_up_casting) SetUpCastingPrimitives();
  PrimitiveVariableType from_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(from)->GetPrimitiveType();
  PrimitiveVariableType to_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(to)->GetPrimitiveType();
  //return can_cast[static_cast<uint8_t>(from_pr)][static_cast<uint8_t>(to_pr)];
  return GetSizeOfPrimitive(from_pr) <= GetSizeOfPrimitive(to_pr);
}

bool CanCastLossless(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValueAsInt(from, to) == 1;
}

bool CanCast(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValueAsInt(from, to) != -1;
}

std::shared_ptr<TIDValue> CastValue(const std::shared_ptr<TIDValue> & val, std::shared_ptr<TIDVariableType> type) {
  if (!type || !val->GetType())
    return std::make_shared<TIDTemporaryValue>(nullptr);
  type = SetParamsToType(type, val->GetType()->IsConst(), val->GetType()->IsReference());
  if (!CanCast(val, type))
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
    return suf.empty() ? PrimitiveVariableType::kF64 : PrimitiveVariableType::kF32;
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

uint64_t value[1 << 16];
void SetUpValue() {
  static bool set_up = false;
  if (!set_up) {
    std::fill(value, value + (1 << 16), -1);
    value[L'0'] = 0;
    value[L'1'] = 1;
    value[L'2'] = 2;
    value[L'3'] = 3;
    value[L'4'] = 4;
    value[L'5'] = 5;
    value[L'6'] = 6;
    value[L'7'] = 7;
    value[L'8'] = 8;
    value[L'9'] = 9;
    value[L'a'] = value[L'A'] = 10;
    value[L'b'] = value[L'B'] = 11;
    value[L'c'] = value[L'C'] = 12;
    value[L'd'] = value[L'D'] = 13;
    value[L'e'] = value[L'E'] = 14;
    value[L'f'] = value[L'F'] = 15;
    set_up = true;
  }
}

uint64_t IntegerFromString(const std::wstring & val, PrimitiveVariableType type) {
  SetUpValue();
  uint64_t num = 0;
  uint64_t base = 10;
  size_t i = 0;
  if (val.size() > 2 && val[0] == L'0' && val[1] == L'x') {
    base = 16;
    i = 2;
  }
  for (; i < val.size(); ++i) {
    num *= base;
    num += value[val[i]];
  }

  switch (type) {
    case PrimitiveVariableType::kInt8:
    case PrimitiveVariableType::kUint8:
      num &= (1ll << 8) - 1;
      break;
    case PrimitiveVariableType::kInt16:
    case PrimitiveVariableType::kUint16:
      num &= (1ll << 16) - 1;
      break;
    case PrimitiveVariableType::kInt32:
    case PrimitiveVariableType::kUint32:
      num &= (1ll << 32) - 1;
      break;
    case PrimitiveVariableType::kInt64:
    case PrimitiveVariableType::kUint64:
      break;
    default:
      assert(false); // :)
  }
  return num;
}

uint64_t DecimalFromString(const std::wstring & val, PrimitiveVariableType type) {
  SetUpValue();
  uint64_t result;
  std::string number;
  number.resize(val.size());
  for (size_t i = 0; i < val.size(); ++i)
    number[i] = static_cast<char>(val[i]);
  if (number.back() == 'f') number.pop_back();
  double num = std::stod(number);
  switch (type) {
    case PrimitiveVariableType::kF32:
      {
        float num_fl = static_cast<float>(num);
        result = *reinterpret_cast<uint32_t*>(&num_fl);
      }
      break;
    case PrimitiveVariableType::kF64:
      result = *reinterpret_cast<uint64_t*>(&num);
      break;
    default:
      assert(false); // :)
  }
  return result;
}

void Cast(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to, RPN & rpn) {
  assert(CanCast(from, to));
  auto from_type = from->GetType();
  bool from_ref = (from->GetValueType() == TIDValueType::kVariable || from_type->IsReference());
  bool to_ref = to->IsReference();
  if (from_ref && to_ref) return;
  PrimitiveVariableType from_t = GetTypeOfVariable(from->GetType());
  PrimitiveVariableType to_t = GetTypeOfVariable(to);
  if (from_ref && !to_ref)
    rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, from_t));
  if (from_t == to_t) return;
  if (from_t == PrimitiveVariableType::kF64)
    rpn.PushNode(RPNOperator(RPNOperatorType::kFromF64, to_t));
  else if (to_t == PrimitiveVariableType::kF64)
    rpn.PushNode(RPNOperator(RPNOperatorType::kToF64, from_t));
  else if (to_t == PrimitiveVariableType::kBool)
    rpn.PushNode(RPNOperator(RPNOperatorType::kToBool, from_t));
  else if (to_t == PrimitiveVariableType::kInt64 || to_t == PrimitiveVariableType::kUint64)
    rpn.PushNode(RPNOperator(RPNOperatorType::kToInt64));
  else if (to_t == PrimitiveVariableType::kF32 || from_t == PrimitiveVariableType::kF32) {
    rpn.PushNode(RPNOperator(RPNOperatorType::kToF64, from_t));
    rpn.PushNode(RPNOperator(RPNOperatorType::kFromF64, to_t));
  } else {
    rpn.PushNode(RPNOperator(RPNOperatorType::kToInt64, from_t));
    rpn.PushNode(RPNOperand((1ull << GetSizeOfPrimitive(to_t)) - 1));
    rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseAnd, PrimitiveVariableType::kUint64));
  }
}
