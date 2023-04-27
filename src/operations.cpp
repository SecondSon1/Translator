#include "operations.hpp"

#include "TID.hpp"
#include "operators.hpp"
#include "exceptions.hpp"
#include "casts.hpp"

std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> unary_operations[kUnaryPrefixOperatorCount + kUnaryPostfixOperatorCount];

bool set_up_unary = false, set_up_binary = false;
void SetUpUnaryOperations() {
  {
    PrimitiveVariableType numeric[] = {
      PrimitiveVariableType::kInt8, PrimitiveVariableType::kUint8, PrimitiveVariableType::kInt16,
      PrimitiveVariableType::kUint16, PrimitiveVariableType::kInt32, PrimitiveVariableType::kUint32,
      PrimitiveVariableType::kInt64, PrimitiveVariableType::kUint64, PrimitiveVariableType::kF32,
      PrimitiveVariableType::kF64, PrimitiveVariableType::kChar
    };
    for (PrimitiveVariableType primitive_type : numeric) {
      // if for some type not defined and it is a reference, not referenced version is also checked
      // if it is not const const version is also checked
      // so if it is const non-reference only it itself will be checked
      // but if it is non-const reference all 4 combinations will be checked (const - non-const, reference - non-reference)
      std::shared_ptr<TIDVariableType> type = SetConstToType(GetPrimitiveVariableType(primitive_type), true);
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kPlus)][type] = type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kMinus)][type] = type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][type] = type;

      std::shared_ptr<TIDVariableType> ref_type = SetReferenceToType(SetConstToType(type, false), true);
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kIncrement)][ref_type] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kDecrement)][ref_type] = ref_type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kIncrement)][ref_type] = type;
      unary_operations[static_cast<uint8_t>(UnaryPostfixOperator::kDecrement)][ref_type] = type;

      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][type] = GetPrimitiveVariableType(PrimitiveVariableType::kBool);
    }
    std::shared_ptr<TIDVariableType> bool_type = GetPrimitiveVariableType(PrimitiveVariableType::kBool);
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][bool_type] = bool_type;
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][bool_type] = bool_type;

    // TODO: add new and delete operators (maybe)
  }

  set_up_unary = true;
}

std::map<BinaryOperator, std::map<std::pair<std::shared_ptr<TIDVariableType>,
  std::shared_ptr<TIDVariableType>>, std::shared_ptr<TIDVariableType>>> binary_operations;

void SetUpBinaryOperations() {
  {
    PrimitiveVariableType non_integer[] = {
      PrimitiveVariableType::kF32, PrimitiveVariableType::kF64
    };
    PrimitiveVariableType integer[] = {
      PrimitiveVariableType::kInt8, PrimitiveVariableType::kUint8, PrimitiveVariableType::kInt16,
      PrimitiveVariableType::kUint16, PrimitiveVariableType::kInt32, PrimitiveVariableType::kUint32,
      PrimitiveVariableType::kInt64, PrimitiveVariableType::kUint64, PrimitiveVariableType::kChar
    };

    BinaryOperator const_ops[] = {
      BinaryOperator::kMultiplication, BinaryOperator::kDivision, BinaryOperator::kModulus, BinaryOperator::kAddition, BinaryOperator::kSubtraction,
      BinaryOperator::kBitwiseShiftLeft, BinaryOperator::kBitwiseShiftRight, BinaryOperator::kLess, BinaryOperator::kMore,
      BinaryOperator::kLessOrEqual, BinaryOperator::kMoreOrEqual, BinaryOperator::kEqual, BinaryOperator::kNotEqual,
      BinaryOperator::kBitwiseAnd, BinaryOperator::kBitwiseOr, BinaryOperator::kBitwiseXor, BinaryOperator::kLogicalAnd,
      BinaryOperator::kLogicalOr
    };
    BinaryOperator const_non_int_ops[] = {
      BinaryOperator::kMultiplication, BinaryOperator::kDivision, BinaryOperator::kModulus, BinaryOperator::kAddition, BinaryOperator::kSubtraction,
      BinaryOperator::kLess, BinaryOperator::kMore, BinaryOperator::kLessOrEqual, BinaryOperator::kMoreOrEqual, BinaryOperator::kEqual,
      BinaryOperator::kNotEqual, BinaryOperator::kLogicalAnd, BinaryOperator::kLogicalOr
    };
    BinaryOperator ref_left_ops[] = {
      BinaryOperator::kAssignment, BinaryOperator::kAdditionAssignment, BinaryOperator::kSubtractionAssignment, BinaryOperator::kMultiplicationAssignment,
      BinaryOperator::kDivisionAssignment, BinaryOperator::kModulusAssignment, BinaryOperator::kBitwiseShiftLeftAssignment,
      BinaryOperator::kBitwiseShiftRightAssignment, BinaryOperator::kBitwiseAndAssignment, BinaryOperator::kBitwiseOr, BinaryOperator::kBitwiseXor
    };
    BinaryOperator ref_left_non_int_ops[] = {
      BinaryOperator::kAssignment, BinaryOperator::kAdditionAssignment, BinaryOperator::kSubtractionAssignment,
      BinaryOperator::kMultiplicationAssignment, BinaryOperator::kDivisionAssignment, BinaryOperator::kModulusAssignment
    };

    for (PrimitiveVariableType primitive_type : integer) {
      std::shared_ptr<TIDVariableType> type = SetConstToType(GetPrimitiveVariableType(primitive_type), true);

      for (BinaryOperator const_op : const_ops)
        binary_operations[const_op][{ type, type }] = type;

      std::shared_ptr<TIDVariableType> ref_type = SetParamsToType(type, false, true);

      for (BinaryOperator ref_left_op : ref_left_ops)
        binary_operations[ref_left_op][{ ref_type, type }] = ref_type;
    }

    for (PrimitiveVariableType primitive_type : non_integer) {
      std::shared_ptr<TIDVariableType> type = SetConstToType(GetPrimitiveVariableType(primitive_type), true);

      for (BinaryOperator const_op : const_non_int_ops)
        binary_operations[const_op][{ type, type }] = type;

      std::shared_ptr<TIDVariableType> ref_type = SetParamsToType(type, false, true);

      for (BinaryOperator ref_left_op : ref_left_non_int_ops)
        binary_operations[ref_left_op][{ ref_type, type }] = ref_type;
    }

    std::shared_ptr<TIDVariableType> bool_type = SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true);
    binary_operations[BinaryOperator::kEqual][{ bool_type, bool_type }] = bool_type;
    binary_operations[BinaryOperator::kNotEqual][{ bool_type, bool_type }] = bool_type;
    binary_operations[BinaryOperator::kLogicalAnd][{ bool_type, bool_type }] = bool_type;
    binary_operations[BinaryOperator::kLogicalOr][{ bool_type, bool_type }] = bool_type;
    binary_operations[BinaryOperator::kBitwiseAnd][{ bool_type, bool_type }] = bool_type;
    binary_operations[BinaryOperator::kBitwiseOr][{ bool_type, bool_type }] = bool_type;
    binary_operations[BinaryOperator::kBitwiseXor][{ bool_type, bool_type }] = bool_type;
    std::shared_ptr<TIDVariableType> ref_type = SetParamsToType(bool_type, false, true);
    binary_operations[BinaryOperator::kAssignment][{ ref_type, bool_type }] = ref_type;
    binary_operations[BinaryOperator::kBitwiseAndAssignment][{ ref_type, bool_type }] = ref_type;
    binary_operations[BinaryOperator::kBitwiseOrAssignment][{ ref_type, bool_type }] = ref_type;
    binary_operations[BinaryOperator::kBitwiseXorAssignment][{ ref_type, bool_type }] = ref_type;

    // TODO: add new and delete operators (maybe)
  }

  set_up_binary = true;
}

std::shared_ptr<TIDVariableType> UnaryOperation(uint8_t op, const std::shared_ptr<TIDVariableType> & type, const Lexeme & lexeme) {
  if (!set_up_unary) SetUpUnaryOperations();
  auto derivable = GetDerivedTypes(type);
  for (auto d_type : derivable)
    if (unary_operations[op].count(type))
      return unary_operations[op][type];

  throw UnknownOperator(lexeme, Operator(UnaryPrefixOperator::kUnknown));
}

std::shared_ptr<TIDVariableType> UnaryPrefixOperation(UnaryPrefixOperator op, const std::shared_ptr<TIDVariableType> & type,
      const Lexeme & lexeme) {
  if (op == UnaryPrefixOperator::kDereference) {
    if (type->GetType() == VariableType::kPointer)
      return std::static_pointer_cast<TIDPointerVariableType>(type)->GetValue();
    else
      throw UnknownOperator(lexeme, op);
  }
  if (op == UnaryPrefixOperator::kAddressOf)
    return DerivePointerFromType(type); // we don't know what lvalue and rvalue are in our lang so ... &5 is correct ig...

  try {
    auto result = UnaryOperation(static_cast<uint8_t>(op), type, lexeme);
    return result;
  } catch (const UnknownOperator & uo) {
    throw UnknownOperator(lexeme, op);
  }
}
std::shared_ptr<TIDVariableType> UnaryPostfixOperation(const std::shared_ptr<TIDVariableType> & type, UnaryPostfixOperator op,
      const Lexeme & lexeme) {
  try {
    auto result = UnaryOperation(static_cast<uint8_t>(op), type, lexeme);
    return result;
  } catch (const UnknownOperator & uo) {
    throw UnknownOperator(lexeme, op);
  }
}

// TODO: check for nullptr
std::shared_ptr<TIDVariableType> BinaryOperation(const std::shared_ptr<TIDVariableType> & lhs,
    BinaryOperator op, const std::shared_ptr<TIDVariableType> & rhs, const Lexeme & lexeme) {
  if (!set_up_binary) SetUpBinaryOperations();
  auto derivable_lhs = GetDerivedTypes(lhs);
  auto derivable_rhs = GetDerivedTypes(rhs);
  auto lct = LeastCommonType(lhs, rhs);
  for (auto d_lhs : derivable_lhs) {
    for (auto d_rhs : derivable_rhs) {
      auto lhs_lct = SetParamsToType(lct, d_lhs->IsConst(), d_lhs->IsReference());
      auto rhs_lct = SetParamsToType(lct, d_rhs->IsConst(), d_rhs->IsReference());
      if (binary_operations[op].count({ lhs_lct, rhs_lct }))
        return binary_operations[op][{ lhs_lct, rhs_lct }];
    }
  }

  throw UnknownOperator(lexeme, op);
}
