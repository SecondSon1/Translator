#include "operators.hpp"

constexpr uint8_t kOperatorCount = 44;
constexpr uint8_t kUnaryPrefixOperatorCount = 10;
constexpr uint8_t kUnaryPostfixOperatorCount = 4;
constexpr uint8_t kBinaryOperatorCount = 30;
std::wstring operator_string[kOperatorCount];
std::map<std::wstring, UnaryPrefixOperator> unary_prefix_operator_by_string;
std::map<std::wstring, UnaryPostfixOperator> unary_postfix_operator_by_string;
std::map<std::wstring, BinaryOperator> binary_operator_by_string;

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
  // ===============
  // === WARNING ===
  // ===============
  // These two must be handled separately, because of this that value doesn't really matter
  // I am adding these for possibility of printing them in debug (s.t. ToString func works)
  Set(kFunctionCall, "()");
  Set(kSubscript, "[]");
#undef current_operator
#undef current_map

#define current_operator BinaryOperator
#define current_map binary_operator_by_string
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
