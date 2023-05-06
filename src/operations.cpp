#include "operations.hpp"

#include "TID.hpp"
#include "operators.hpp"
#include "exceptions.hpp"
#include "casts.hpp"
#include <memory>

typedef std::pair<TIDValueType, std::shared_ptr<TIDVariableType>> VariableTypeWithValue;
constexpr TIDValueType value_types[] = { TIDValueType::kTemporary, TIDValueType::kVariable };

std::map<VariableTypeWithValue, std::shared_ptr<TIDVariableType>> unary_operations[kUnaryPrefixOperatorCount + kUnaryPostfixOperatorCount];

constexpr PrimitiveVariableType numeric[] = {
  PrimitiveVariableType::kInt8, PrimitiveVariableType::kUint8, PrimitiveVariableType::kInt16,
  PrimitiveVariableType::kUint16, PrimitiveVariableType::kInt32, PrimitiveVariableType::kUint32,
  PrimitiveVariableType::kInt64, PrimitiveVariableType::kUint64, PrimitiveVariableType::kF32,
  PrimitiveVariableType::kF64, PrimitiveVariableType::kChar
};
bool IsTypeInteger(PrimitiveVariableType type) {
  return type != PrimitiveVariableType::kF32 && type != PrimitiveVariableType::kF64 && type != PrimitiveVariableType::kBool;
}

bool set_up_unary = false, set_up_binary = false;
void SetUpUnaryOperations() {
  {
    std::shared_ptr<TIDVariableType> bool_type = GetPrimitiveVariableType(PrimitiveVariableType::kBool);
    std::shared_ptr<TIDVariableType> const_bool_type = SetConstToType(bool_type, true);

    for (PrimitiveVariableType primitive_type : numeric) {
      // if for some type not defined and it is a reference, not referenced version is also checked
      // if it is not const const version is also checked
      // so if it is const non-reference only it itself will be checked
      // but if it is non-const reference all 4 combinations will be checked (const - non-const, reference - non-reference)
      std::shared_ptr<TIDVariableType> type = GetPrimitiveVariableType(primitive_type);
      std::shared_ptr<TIDVariableType> const_type = SetConstToType(type, true);
      for (auto val_type : value_types) {
        unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kPlus)][{ val_type, const_type }] = const_type;
        unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kMinus)][{ val_type, const_type }] = const_type;

        if (primitive_type != PrimitiveVariableType::kF32 && primitive_type != PrimitiveVariableType::kF64)
          unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][{ val_type, const_type }] = const_type;
      }

      std::shared_ptr<TIDVariableType> ref_type = SetParamsToType(type, false, true);
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kIncrement)][{ TIDValueType::kVariable, type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kIncrement)][{ TIDValueType::kTemporary, ref_type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kDecrement)][{ TIDValueType::kVariable, type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kDecrement)][{ TIDValueType::kTemporary, ref_type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kIncrement)][{ TIDValueType::kVariable, type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kIncrement)][{ TIDValueType::kTemporary, ref_type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kDecrement)][{ TIDValueType::kVariable, type }] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kDecrement)][{ TIDValueType::kTemporary, ref_type }] = ref_type;

      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][{ TIDValueType::kVariable, type }] = const_bool_type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][{ TIDValueType::kTemporary, type }] = const_bool_type;
    }

    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][{ TIDValueType::kVariable, const_bool_type }] = const_bool_type;
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][{ TIDValueType::kTemporary, const_bool_type }] = const_bool_type;
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][{ TIDValueType::kVariable, const_bool_type }] = const_bool_type;
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][{ TIDValueType::kTemporary, const_bool_type }] = const_bool_type;
  }

  set_up_unary = true;
}

constexpr BinaryOperator arithmetic_ops[] = {
  BinaryOperator::kMultiplication, BinaryOperator::kDivision, BinaryOperator::kModulus,
  BinaryOperator::kAddition, BinaryOperator::kSubtraction,
};
constexpr BinaryOperator bitwise_ops[] = {
  BinaryOperator::kBitwiseShiftLeft, BinaryOperator::kBitwiseShiftRight, BinaryOperator::kBitwiseAnd,
  BinaryOperator::kBitwiseOr, BinaryOperator::kBitwiseXor
};
constexpr BinaryOperator assignment_arithmetic_ops[] = {
  BinaryOperator::kAssignment, BinaryOperator::kAdditionAssignment, BinaryOperator::kSubtractionAssignment,
  BinaryOperator::kMultiplicationAssignment, BinaryOperator::kDivisionAssignment, BinaryOperator::kModulusAssignment
};
constexpr BinaryOperator assignment_bitwise_ops[] = {
  BinaryOperator::kBitwiseShiftLeftAssignment, BinaryOperator::kBitwiseShiftRightAssignment,
  BinaryOperator::kBitwiseAndAssignment, BinaryOperator::kBitwiseOrAssignment, BinaryOperator::kBitwiseXorAssignment
};
constexpr BinaryOperator logical_ops[] = {
  BinaryOperator::kEqual, BinaryOperator::kNotEqual, BinaryOperator::kLess, BinaryOperator::kLessOrEqual,
  BinaryOperator::kMore, BinaryOperator::kMoreOrEqual, BinaryOperator::kLogicalAnd, BinaryOperator::kLogicalOr
};
std::map<BinaryOperator, std::map<std::pair<VariableTypeWithValue, VariableTypeWithValue>,
                                  std::shared_ptr<TIDVariableType>>> binary_operations;

void SetUpBinaryOperations() {
  {
    std::shared_ptr<TIDVariableType> bool_type = GetPrimitiveVariableType(PrimitiveVariableType::kBool);
    std::shared_ptr<TIDVariableType> const_bool_type = SetConstToType(bool_type, true);

    for (PrimitiveVariableType primitive_type : numeric) {
      std::shared_ptr<TIDVariableType> type = GetPrimitiveVariableType(primitive_type);
      std::shared_ptr<TIDVariableType> const_type = SetConstToType(type, true);

      for (BinaryOperator op : arithmetic_ops) {
        for (auto lhs_val_type : value_types)
          for (auto rhs_val_type : value_types)
            binary_operations[op][{ { lhs_val_type, const_type }, { rhs_val_type, const_type } }] = type;
      }

      for (BinaryOperator op : logical_ops) {
        for (auto lhs_val_type : value_types)
          for (auto rhs_val_type : value_types)
            binary_operations[op][{ { lhs_val_type, const_type }, { rhs_val_type, const_type } }] = const_bool_type;
      }

      if (IsTypeInteger(primitive_type)) {
        for (BinaryOperator op : bitwise_ops) {
          for (auto lhs_val_type : value_types)
            for (auto rhs_val_type : value_types)
              binary_operations[op][{ { lhs_val_type, const_type }, { rhs_val_type, const_type } }] = const_bool_type;
        }
      }

      std::shared_ptr<TIDVariableType> ref_type = SetParamsToType(type, false, true);

      for (BinaryOperator op : assignment_arithmetic_ops) {
        for (auto rhs_val_type : value_types) {
          binary_operations[op][{ { TIDValueType::kVariable, type }, { rhs_val_type, const_type } }] = ref_type;
          binary_operations[op][{ { TIDValueType::kTemporary, ref_type }, { rhs_val_type, const_type } }] = ref_type;
        }
      }

      if (IsTypeInteger(primitive_type)) {
        for (BinaryOperator op : assignment_bitwise_ops) {
          for (auto rhs_val_type : value_types) {
            binary_operations[op][{ { TIDValueType::kVariable, type }, { rhs_val_type, const_type } }] = ref_type;
            binary_operations[op][{ { TIDValueType::kTemporary, ref_type }, { rhs_val_type, const_type } }] = ref_type;
          }
        }
      }
    }

    for (BinaryOperator op : logical_ops) {
      for (auto lhs_val_type : value_types)
        for (auto rhs_val_type : value_types)
          binary_operations[op][{ { lhs_val_type, const_bool_type }, { rhs_val_type, const_bool_type } }] = const_bool_type;
    }
    std::shared_ptr<TIDVariableType> ref_bool_type = SetParamsToType(bool_type, false, true);
    for (BinaryOperator op : assignment_bitwise_ops) {
      for (auto rhs_val_type : value_types) {
        binary_operations[op][{ { TIDValueType::kVariable, bool_type }, { rhs_val_type, const_bool_type } }] = ref_bool_type;
        binary_operations[op][{ { TIDValueType::kTemporary, ref_bool_type }, { rhs_val_type, const_bool_type } }] = ref_bool_type;
      }
    }
  }

  set_up_binary = true;
}

std::shared_ptr<TIDValue> UnaryOperation(uint8_t op, const std::shared_ptr<TIDValue> & value, const Lexeme & lexeme) {
  if (!set_up_unary) SetUpUnaryOperations();
  auto type = value->GetType();
  auto derivable = GetDerivedTypes(value);
  for (auto d_type : derivable) {
    VariableTypeWithValue type_with_value = { d_type->GetValueType(), d_type->GetType() };
    if (unary_operations[op].count(type_with_value))
      return std::make_shared<TIDTemporaryValue>(unary_operations[op][type_with_value]);
  }

  throw UnknownOperator(lexeme, UnaryPrefixOperator::kUnknown, nullptr);
}

std::shared_ptr<TIDValue> UnaryPrefixOperation(UnaryPrefixOperator op, const std::shared_ptr<TIDValue> & val,
      const Lexeme & lexeme) {
  auto type = val->GetType();
  if (op == UnaryPrefixOperator::kDereference) {
    if (type->GetType() == VariableType::kPointer) {
      auto val_type = std::static_pointer_cast<TIDPointerVariableType>(type)->GetValue();
      return std::make_shared<TIDTemporaryValue>(SetParamsToType(val_type, type->IsConst(), true));
    } else
      throw UnknownOperator(lexeme, op, val);
  }
  if (op == UnaryPrefixOperator::kAddressOf) {
    if (val->GetValueType() == TIDValueType::kTemporary && !type->IsReference())
      throw UnknownOperator(lexeme, op, val);
    return std::make_shared<TIDTemporaryValue>(DerivePointerFromType(type));
  }

  try {
    auto result = UnaryOperation(static_cast<uint8_t>(op), val, lexeme);
    return result;
  } catch (const UnknownOperator & uo) {
    throw UnknownOperator(lexeme, op, val);
  }
}
std::shared_ptr<TIDValue> UnaryPostfixOperation(const std::shared_ptr<TIDValue> & val, UnaryPostfixOperator op,
      const Lexeme & lexeme) {
  try {
    auto result = UnaryOperation(static_cast<uint8_t>(op), val, lexeme);
    return result;
  } catch (const UnknownOperator & uo) {
    throw UnknownOperator(lexeme, op, val);
  }
}

// TODO: check for assignment operations separately
std::shared_ptr<TIDValue> BinaryOperation(const std::shared_ptr<TIDValue> & lhs,
    BinaryOperator op, const std::shared_ptr<TIDValue> & rhs, const Lexeme & lexeme) {
  if (!set_up_binary) SetUpBinaryOperations();

  auto lhs_type = lhs->GetType();
  auto rhs_type = rhs->GetType();
  if (!lhs_type || !rhs_type)
    throw UnknownOperator(lexeme, op, lhs, rhs);

  if (lhs_type->GetType() != VariableType::kPrimitive || rhs_type->GetType() != VariableType::kPrimitive) {
    if (op == BinaryOperator::kAssignment) {
      auto lhs_type_clean = SetParamsToType(lhs_type, false, false);
      auto rhs_type_clean = SetParamsToType(rhs_type, false, false);
      if (lhs_type_clean == rhs_type_clean && !lhs_type->IsConst() && (lhs->GetValueType() == TIDValueType::kVariable || lhs_type->IsReference()))
        return std::make_shared<TIDTemporaryValue>(SetParamsToType(lhs_type, false, true));
    }
    std::shared_ptr<TIDVariableType> int64_type = SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kInt64), true);
    if (lhs_type->GetType() == rhs_type->GetType() && lhs_type->GetType() == VariableType::kPointer && op == BinaryOperator::kSubtraction) {
      return std::make_shared<TIDTemporaryValue>(int64_type);
    } else if (op == BinaryOperator::kAddition || op == BinaryOperator::kAdditionAssignment) {
      if (op == BinaryOperator::kAdditionAssignment) {
        if (lhs_type->GetType() == VariableType::kPointer && CanCast(rhs_type, int64_type)
            && !lhs_type->IsConst() && (lhs_type->IsReference() || lhs->GetValueType() == TIDValueType::kVariable))
          return std::make_shared<TIDTemporaryValue>(SetParamsToType(lhs_type, false, true));
      } else {
        if (lhs_type->GetType() == VariableType::kPointer && CanCast(rhs_type, int64_type))
          return std::make_shared<TIDTemporaryValue>(SetParamsToType(lhs_type, true, false));
        else if (rhs_type->GetType() == VariableType::kPointer && CanCast(lhs_type, int64_type))
          return std::make_shared<TIDTemporaryValue>(SetParamsToType(rhs_type, true, false));
      }
    }
    throw UnknownOperator(lexeme, op, lhs, rhs);
  }

  bool assignment = false;
  for (BinaryOperator assignment_op : assignment_arithmetic_ops)
    assignment |= op == assignment_op;
  for (BinaryOperator assignment_op : assignment_bitwise_ops)
    assignment |= op == assignment_op;

  PrimitiveVariableType lhs_prim = std::static_pointer_cast<TIDPrimitiveVariableType>(lhs_type)->GetPrimitiveType();
  PrimitiveVariableType rhs_prim = std::static_pointer_cast<TIDPrimitiveVariableType>(rhs_type)->GetPrimitiveType();


  if (assignment) {
    std::shared_ptr<TIDValue> rhs_casted_val = CastValue(rhs, lhs_type);
    std::shared_ptr<TIDVariableType> rhs_casted_type = rhs_casted_val->GetType();
    auto derivable_lhs = GetDerivedTypes(lhs);
    auto derivable_rhs = GetDerivedTypes(rhs_casted_val);
    for (auto d_lhs : derivable_lhs) {
      for (auto d_rhs : derivable_rhs) {
        auto dt_lhs = d_lhs->GetType();
        auto dt_rhs = d_rhs->GetType();

        std::pair<VariableTypeWithValue, VariableTypeWithValue> args = {
          { d_lhs->GetValueType(), dt_lhs }, { d_rhs->GetValueType(), dt_rhs }
        };
        if (binary_operations[op].count(args))
          return std::make_shared<TIDTemporaryValue>(binary_operations[op][args]);
      }
    }
  } else {
    std::shared_ptr<TIDVariableType> lct = GetPrimitiveVariableType(LeastCommonType(lhs_prim, rhs_prim));
    std::shared_ptr<TIDValue> lhs_casted = CastValue(lhs, lct), rhs_casted = CastValue(rhs, lct);
    auto derivable_lhs = GetDerivedTypes(lhs_casted);
    auto derivable_rhs = GetDerivedTypes(rhs_casted);
    for (auto d_lhs : derivable_lhs) {
      for (auto d_rhs : derivable_rhs) {
        auto dt_lhs = d_lhs->GetType();
        auto dt_rhs = d_rhs->GetType();

        std::pair<VariableTypeWithValue, VariableTypeWithValue> args = {
          { d_lhs->GetValueType(), dt_lhs }, { d_rhs->GetValueType(), dt_rhs }
        };
        if (binary_operations[op].count(args))
          return std::make_shared<TIDTemporaryValue>(binary_operations[op][args]);
      }
    }
  }

  throw UnknownOperator(lexeme, op, lhs, rhs);
}
