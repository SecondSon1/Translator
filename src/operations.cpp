#include "operations.hpp"

#include "TID.hpp"
#include "generation.hpp"
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
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kIncrement)][{ TIDValueType::kVariable, type }] = const_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kIncrement)][{ TIDValueType::kTemporary, ref_type }] = const_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kDecrement)][{ TIDValueType::kVariable, type }] = const_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kDecrement)][{ TIDValueType::kTemporary, ref_type }] = const_type;

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
      return std::make_shared<TIDTemporaryValue>(SetReferenceToType(val_type, true));
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

std::shared_ptr<TIDValue> BinaryOperationRPN(const std::shared_ptr<TIDValue> & lhs,
    BinaryOperator op, const std::shared_ptr<TIDValue> & rhs, const Lexeme & lexeme, RPN & rpn) {
  if (!set_up_binary) SetUpBinaryOperations();

  auto lhs_type = lhs->GetType();
  auto rhs_type = rhs->GetType();
  if (!lhs_type || !rhs_type)
    throw UnknownOperator(lexeme, op, lhs, rhs);

  if (lhs_type->GetType() != VariableType::kPrimitive || rhs_type->GetType() != VariableType::kPrimitive) {
    if (op == BinaryOperator::kAssignment) {
      auto lhs_type_clean = SetParamsToType(lhs_type, false, false);
      auto rhs_type_clean = SetParamsToType(rhs_type, false, false);
      bool types_eq;
      if (lhs_type->GetType() == VariableType::kPointer && rhs_type->GetType() == VariableType::kPointer)
        types_eq = true;
      else
        types_eq = lhs_type_clean == rhs_type_clean;
      if (types_eq && !lhs_type->IsConst() && IsReference(lhs)) {
        if (lhs_type->GetType() == VariableType::kComplex) {
          // complex is always an address
          // FROM TO SIZE OP::COPY; from = rhs; to = lhs; size = sizeof(struct)
          rpn.PushNode(RPNOperand(lhs_type->GetSize()));
          rpn.PushNode(RPNOperator(RPNOperatorType::kCopyTF));
        } else {
          Cast(rhs, SetParamsToType(rhs_type, true, false), rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, GetTypeOfVariable(lhs_type)));
        }
        return std::make_shared<TIDTemporaryValue>(SetParamsToType(lhs_type, false, true));
      }
    }
    std::shared_ptr<TIDVariableType> int64_type = SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kInt64), true);
    if (lhs_type->GetType() == rhs_type->GetType() && lhs_type->GetType() == VariableType::kPointer && op == BinaryOperator::kSubtraction) {
      rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
      LoadIfReference(lhs, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
      LoadIfReference(rhs, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kSubtract, PrimitiveVariableType::kInt64));
      return std::make_shared<TIDTemporaryValue>(int64_type);
    } else if (op == BinaryOperator::kAddition || op == BinaryOperator::kAdditionAssignment) {
      if (op == BinaryOperator::kAdditionAssignment) {
        if (lhs_type->GetType() == VariableType::kPointer && CanCast(rhs, int64_type)
            && !lhs_type->IsConst() && (lhs_type->IsReference() || lhs->GetValueType() == TIDValueType::kVariable)) {
          rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
          rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
          rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
          LoadIfReference(lhs, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
          Cast(rhs, int64_type, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
          rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, PrimitiveVariableType::kUint64));
          return std::make_shared<TIDTemporaryValue>(SetParamsToType(lhs_type, false, true));
        }
      } else {
        if (lhs_type->GetType() == VariableType::kPointer && CanCast(rhs, int64_type)) {
          rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
          LoadIfReference(lhs, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
          Cast(rhs, int64_type, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
          return std::make_shared<TIDTemporaryValue>(SetParamsToType(lhs_type, true, false));
        } else if (rhs_type->GetType() == VariableType::kPointer && CanCast(lhs, int64_type)) {
          rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
          Cast(rhs, int64_type, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
          LoadIfReference(lhs, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
          return std::make_shared<TIDTemporaryValue>(SetParamsToType(rhs_type, true, false));
        }
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
        if (binary_operations[op].count(args)) {
          // d_lhs is reference, d_rhs - dunno
          rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
          rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
          rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
          LoadIfReference(d_lhs, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
          Cast(rhs, dt_rhs, rpn);
          LoadIfReference(dt_rhs, rpn);

          switch (op) {
            case BinaryOperator::kAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
              rpn.PushNode(RPNOperator(RPNOperatorType::kDump));
              rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
              break;
            case BinaryOperator::kAdditionAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, lhs_prim));
              break;
            case BinaryOperator::kSubtractionAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kSubtract, lhs_prim));
              break;
            case BinaryOperator::kMultiplicationAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kMultiply, lhs_prim));
              break;
            case BinaryOperator::kDivisionAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kDivide, lhs_prim));
              break;
            case BinaryOperator::kModulusAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kModulus, lhs_prim));
              break;
            case BinaryOperator::kBitwiseShiftLeftAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseShiftLeft, lhs_prim));
              break;
            case BinaryOperator::kBitwiseShiftRightAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseShiftRight, lhs_prim));
              break;
            case BinaryOperator::kBitwiseAndAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseAnd, lhs_prim));
              break;
            case BinaryOperator::kBitwiseOrAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseOr, lhs_prim));
              break;
            case BinaryOperator::kBitwiseXorAssignment:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseXor, lhs_prim));
              break;
            default: break;
          }

          rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, lhs_prim));
          return std::make_shared<TIDTemporaryValue>(binary_operations[op][args]);
        }
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
        lhs_prim = std::dynamic_pointer_cast<TIDPrimitiveVariableType>(d_lhs)->GetPrimitiveType();
        if (binary_operations[op].count(args)) {
          rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
          Cast(lhs, dt_lhs, rpn);
          LoadIfReference(dt_lhs, rpn);
          rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
          Cast(rhs, dt_rhs, rpn);
          LoadIfReference(dt_rhs, rpn);

          switch (op) {
            case BinaryOperator::kAddition:
              rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, lhs_prim));
              break;
            case BinaryOperator::kSubtraction:
              rpn.PushNode(RPNOperator(RPNOperatorType::kSubtract, lhs_prim));
              break;
            case BinaryOperator::kMultiplication:
              rpn.PushNode(RPNOperator(RPNOperatorType::kMultiply, lhs_prim));
              break;
            case BinaryOperator::kDivision:
              rpn.PushNode(RPNOperator(RPNOperatorType::kDivide, lhs_prim));
              break;
            case BinaryOperator::kModulus:
              rpn.PushNode(RPNOperator(RPNOperatorType::kModulus, lhs_prim));
              break;
            case BinaryOperator::kBitwiseShiftLeft:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseShiftLeft, lhs_prim));
              break;
            case BinaryOperator::kBitwiseShiftRight:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseShiftRight, lhs_prim));
              break;
            case BinaryOperator::kBitwiseAnd:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseAnd, lhs_prim));
              break;
            case BinaryOperator::kBitwiseOr:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseOr, lhs_prim));
              break;
            case BinaryOperator::kBitwiseXor:
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseXor, lhs_prim));
              break;
            case BinaryOperator::kEqual:
              rpn.PushNode(RPNOperator(RPNOperatorType::kEqual, lhs_prim));
              break;
            case BinaryOperator::kNotEqual:
              rpn.PushNode(RPNOperator(RPNOperatorType::kNotEqual, lhs_prim));
              break;
            case BinaryOperator::kLess:
              rpn.PushNode(RPNOperator(RPNOperatorType::kLess, lhs_prim));
              break;
            case BinaryOperator::kMore:
              rpn.PushNode(RPNOperator(RPNOperatorType::kMore, lhs_prim));
              break;
            case BinaryOperator::kLessOrEqual:
              rpn.PushNode(RPNOperator(RPNOperatorType::kLessOrEqual, lhs_prim));
              break;
            case BinaryOperator::kMoreOrEqual:
              rpn.PushNode(RPNOperator(RPNOperatorType::kMoreOrEqual, lhs_prim));
              break;
            case BinaryOperator::kLogicalAnd:
              rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
              rpn.PushNode(RPNOperator(RPNOperatorType::kToBool, lhs_prim));
              rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
              rpn.PushNode(RPNOperator(RPNOperatorType::kToBool, lhs_prim));
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseAnd, PrimitiveVariableType::kBool));
              break;
            case BinaryOperator::kLogicalOr:
              rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
              rpn.PushNode(RPNOperator(RPNOperatorType::kToBool, lhs_prim));
              rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
              rpn.PushNode(RPNOperator(RPNOperatorType::kToBool, lhs_prim));
              rpn.PushNode(RPNOperator(RPNOperatorType::kBitwiseOr, PrimitiveVariableType::kBool));
              break;
            default: break;
          }

          return std::make_shared<TIDTemporaryValue>(binary_operations[op][args]);
        }
      }
    }
  }

  throw UnknownOperator(lexeme, op, lhs, rhs);
}
std::shared_ptr<TIDValue> UnaryPrefixOperationRPN(UnaryPrefixOperator op,
    const std::shared_ptr<TIDValue> & val, const Lexeme & lexeme, RPN & rpn) {
  std::shared_ptr<TIDValue> result = UnaryPrefixOperation(op, val, lexeme);
  PrimitiveVariableType primitive_type = GetTypeOfVariable(val->GetType());
  switch (op) {
    case UnaryPrefixOperator::kIncrement: // must be reference
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, primitive_type));
      rpn.PushNode(RPNOperand(1));
      rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, primitive_type));
      rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, primitive_type));
      break;
    case UnaryPrefixOperator::kDecrement:
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, primitive_type));
      rpn.PushNode(RPNOperand(1));
      rpn.PushNode(RPNOperator(RPNOperatorType::kSubtract, primitive_type));
      rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, primitive_type));
      break;
    case UnaryPrefixOperator::kPlus:
      // Nothing lol
      break;
    case UnaryPrefixOperator::kMinus:
      LoadIfReference(val, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kMinus, GetTypeOfVariable(val->GetType())));
      break;
    case UnaryPrefixOperator::kInvert:
      LoadIfReference(val, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kInvert, GetTypeOfVariable(val->GetType())));
      break;
    case UnaryPrefixOperator::kTilda:
      LoadIfReference(val, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kTilda, GetTypeOfVariable(val->GetType())));
      break;
    case UnaryPrefixOperator::kDereference:
      assert(IsReference(val));
      LoadIfReference(val, rpn);
      break;
    case UnaryPrefixOperator::kAddressOf:
      assert(IsReference(val));
      break;
    case UnaryPrefixOperator::kUnknown:
      throw UnknownOperator(lexeme, op, val);
  }
  return result;
}
std::shared_ptr<TIDValue> UnaryPostfixOperationRPN(const std::shared_ptr<TIDValue> & val,
    UnaryPostfixOperator op, const Lexeme & lexeme, RPN & rpn) {
  std::shared_ptr<TIDValue> result = UnaryPostfixOperation(val, op, lexeme);
  PrimitiveVariableType primitive_type = GetTypeOfVariable(val->GetType());
  switch (op) {
    case UnaryPostfixOperator::kIncrement:
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, primitive_type));
      rpn.PushNode(RPNOperand(1));
      rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, primitive_type));
      rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, primitive_type));

      rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, primitive_type));
      rpn.PushNode(RPNOperand(1));
      rpn.PushNode(RPNOperator(RPNOperatorType::kSubtract, primitive_type));
      break;
    case UnaryPostfixOperator::kDecrement:
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, primitive_type));
      rpn.PushNode(RPNOperand(1));
      rpn.PushNode(RPNOperator(RPNOperatorType::kSubtract, primitive_type));
      rpn.PushNode(RPNOperator(RPNOperatorType::kStoreAD, primitive_type));

      rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, primitive_type));
      rpn.PushNode(RPNOperand(1));
      rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, primitive_type));
      break;
    case UnaryPostfixOperator::kUnknown:
      throw UnknownOperator(lexeme, op, val);
  }
  return result;
}
/*
std::shared_ptr<TIDValue> BinaryOperationRPN(const std::shared_ptr<TIDValue> & lhs, BinaryOperator op,
    const std::shared_ptr<TIDValue> & rhs, const Lexeme & lexeme, RPN & rpn) {
  std::shared_ptr<TIDValue> result = BinaryOperation(lhs, op, rhs, lexeme);
  PrimitiveVariableType lhs_primitive = GetTypeOfVariable(lhs->GetType());
  PrimitiveVariableType rhs_primitive = GetTypeOfVariable(rhs->GetType());

  switch (op) {
    case BinaryOperator::kFunctionCall:
    case BinaryOperator::kSubscript:
    case BinaryOperator::kMemberAccess:
      assert(false);
      break;
    case BinaryOperator::kMultiplication:
      rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
      LoadIfReference(lhs, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
      LoadIfReference(rhs, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kMultiply));
      break;
    case BinaryOperator::kDivision:
      rpn.PushNode(RPNOperator(RPNOperatorType::kSave));
      LoadIfReference(lhs, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kRestore));
      LoadIfReference(rhs, rpn);
      rpn.PushNode(RPNOperator(RPNOperatorType::kMultiply));
      break;
    case BinaryOperator::kModulus:
      break;
    case BinaryOperator::kAddition:
      break;
    case BinaryOperator::kSubtraction:
      break;
    case BinaryOperator::kBitwiseShiftLeft:
      break;
    case BinaryOperator::kBitwiseShiftRight:
      break;
    case BinaryOperator::kLess:
      break;
    case BinaryOperator::kMore:
      break;
    case BinaryOperator::kLessOrEqual:
      break;
    case BinaryOperator::kMoreOrEqual:
      break;
    case BinaryOperator::kEqual:
      break;
    case BinaryOperator::kNotEqual:
      break;
    case BinaryOperator::kBitwiseAnd:
      break;
    case BinaryOperator::kBitwiseXor:
      break;
    case BinaryOperator::kBitwiseOr:
      break;
    case BinaryOperator::kLogicalAnd:
      break;
    case BinaryOperator::kLogicalOr:
      break;
    case BinaryOperator::kAssignment:
      break;
    case BinaryOperator::kAdditionAssignment:
      break;
    case BinaryOperator::kSubtractionAssignment:
      break;
    case BinaryOperator::kMultiplicationAssignment:
      break;
    case BinaryOperator::kDivisionAssignment:
      break;
    case BinaryOperator::kModulusAssignment:
      break;
    case BinaryOperator::kBitwiseShiftLeftAssignment:
      break;
    case BinaryOperator::kBitwiseShiftRightAssignment:
      break;
    case BinaryOperator::kBitwiseAndAssignment:
      break;
    case BinaryOperator::kBitwiseOrAssignment:
      break;
    case BinaryOperator::kBitwiseXorAssignment:
      break;
    case BinaryOperator::kUnknown:
      throw UnknownOperator(lexeme, op, lhs, rhs);
  }
  return result;
}*/
