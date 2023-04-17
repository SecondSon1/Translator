#include "operators.hpp"

#include "TID.hpp"
#include "exceptions.hpp"
#include <memory>

constexpr uint8_t kOperatorCount = 44;
constexpr uint8_t kUnaryPrefixOperatorCount = 10;
constexpr uint8_t kUnaryPostfixOperatorCount = 2;
constexpr uint8_t kBinaryOperatorCount = 32;
std::wstring operator_string[kOperatorCount];
std::map<std::wstring, UnaryPrefixOperator> unary_prefix_operator_by_string;
std::map<std::wstring, UnaryPostfixOperator> unary_postfix_operator_by_string;
std::map<std::wstring, BinaryOperator> binary_operator_by_string;

UnaryPrefixOperator Operator::GetUnaryPrefixOperator() const {
  if (type_ != OperatorType::kUnaryPrefix)
    throw OperatorTypeMismatch(OperatorType::kUnaryPrefix, type_);
  return u_pre_op_;
}
UnaryPostfixOperator Operator::GetUnaryPostfixOperator() const {
  if (type_ != OperatorType::kUnaryPostfix)
    throw OperatorTypeMismatch(OperatorType::kUnaryPostfix, type_);
  return u_post_op_;
}
BinaryOperator Operator::GetBinaryOperator() const {
  if (type_ != OperatorType::kBinary)
    throw OperatorTypeMismatch(OperatorType::kBinary, type_);
  return bin_op_;
}


bool set_up = false;
void SetUpOperatorString() {
#define Set(op, value) operator_string[static_cast<uint8_t>(current_operator::op)] = L"" value;\
  current_map[L"" value] = current_operator::op

#define current_operator UnaryPrefixOperator
#define current_map unary_prefix_operator_by_string
  Set(kIncrement, "++");
  Set(kDecrement, "--");
  Set(kPlus, "+");
  Set(kMinus, "-");
  Set(kInvert, "!");
  Set(kTilda, "~");
  Set(kDereference, "*");
  Set(kAddressOf, "&");
  Set(kNew, "new");
  Set(kDelete, "delete");
#undef current_operator
#undef current_map

#define current_operator UnaryPostfixOperator
#define current_map unary_postfix_operator_by_string
  Set(kIncrement, "++");
  Set(kDecrement, "--");
#undef current_operator
#undef current_map

#define current_operator BinaryOperator
#define current_map binary_operator_by_string
  // ===============
  // === WARNING ===
  // ===============
  // These two must be handled separately, because of this that value doesn't really matter
  // I am adding these for possibility of printing them in debug (s.t. ToString func works)
  Set(kFunctionCall, "()");
  Set(kSubscript, "[]");

  Set(kMemberAccess, ".");
  Set(kMultiplication, "*");
  Set(kDivision, "/");
  Set(kModulus, "%");
  Set(kAddition, "+");
  Set(kSubtraction, "-");
  Set(kBitwiseShiftLeft, "<<");
  Set(kBitwiseShiftRight, ">>");
  Set(kLess, "<");
  Set(kMore, ">");
  Set(kLessOrEqual, "<=");
  Set(kMoreOrEqual, ">=");
  Set(kEqual, "==");
  Set(kNotEqual, "!=");
  Set(kBitwiseAnd, "&");
  Set(kBitwiseXor, "^");
  Set(kBitwiseOr, "|");
  Set(kLogicalAnd, "&&");
  Set(kLogicalOr, "||");
  Set(kAssignment, "=");
  Set(kAdditionAssignment, "+=");
  Set(kSubtractionAssignment, "-=");
  Set(kMultiplicationAssignment, "*=");
  Set(kDivisionAssignment, "/=");
  Set(kModulusAssignment, "%=");
  Set(kBitwiseShiftLeftAssignment, "<<=");
  Set(kBitwiseShiftRightAssignment, ">>=");
  Set(kBitwiseAndAssignment, "&=");
  Set(kBitwiseOrAssignment, "|=");
  Set(kBitwiseXorAssignment, "^=");
#undef current_operator
#undef current_map

#undef Set

  set_up = true;
}

std::wstring ToString(UnaryPrefixOperator op) {
  if (!set_up) SetUpOperatorString();
  if (op == UnaryPrefixOperator::kUnknown) return L"";
  return operator_string[static_cast<uint8_t>(op)];
}
std::wstring ToString(UnaryPostfixOperator op) {
  if (!set_up) SetUpOperatorString();
  if (op == UnaryPostfixOperator::kUnknown) return L"";
  return operator_string[static_cast<uint8_t>(op)];
}
std::wstring ToString(BinaryOperator op) {
  if (!set_up) SetUpOperatorString();
  if (op == BinaryOperator::kUnknown) return L"";
  return operator_string[static_cast<uint8_t>(op)];
}

UnaryPrefixOperator UnaryPrefixOperatorByString(const std::wstring & str) {
  if (!set_up) SetUpOperatorString();
  if (unary_prefix_operator_by_string.count(str))
    return unary_prefix_operator_by_string[str];
  else
    return UnaryPrefixOperator::kUnknown;
}
UnaryPostfixOperator UnaryPostfixOperatorByString(const std::wstring & str) {
  if (!set_up) SetUpOperatorString();
  if (unary_postfix_operator_by_string.count(str))
    return unary_postfix_operator_by_string[str];
  else
    return UnaryPostfixOperator::kUnknown;
}
BinaryOperator BinaryOperatorByString(const std::wstring & str) {
  if (!set_up) SetUpOperatorString();
  if (binary_operator_by_string.count(str))
    return binary_operator_by_string[str];
  else
    return BinaryOperator::kUnknown;
}

std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> unary_operations[kUnaryPrefixOperatorCount + kUnaryPostfixOperatorCount];

bool set_up_unary = false;
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
      std::shared_ptr<TIDVariableType> type = GetPrimitiveVariableType(primitive_type);
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kPlus)][type] = type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kMinus)][type] = type;
      unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][type] = type;

      std::shared_ptr<TIDVariableType> ref_type = SetReferenceToType(type, true);
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

std::shared_ptr<TIDVariableType> UnaryOperation(uint8_t op, const std::shared_ptr<TIDVariableType> & type, const Lexeme & lexeme) {
  if (!set_up_unary) SetUpUnaryOperations();
  if (unary_operations[op].count(type))
    return unary_operations[op][type];

  if (type->IsReference()) {
    auto x = SetReferenceToType(type, false);
    if (unary_operations[op].count(x))
      return unary_operations[op][x];
  }

  if (!type->IsConst()) {
    auto x = SetConstToType(type, true);
    if (unary_operations[op].count(x))
      return unary_operations[op][x];
  }
  if (type->IsReference() && !type->IsConst()) {
    auto x = SetConstToType(type, true);
    x = SetReferenceToType(type, false);
    if (unary_operations[op].count(x))
      return unary_operations[op][x];
  }
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
std::shared_ptr<TIDVariableType> BinaryOperation(const std::shared_ptr<TIDVariableType> & lhs,
    BinaryOperator op, const std::shared_ptr<TIDVariableType> & rhs) {
  return {};
}
