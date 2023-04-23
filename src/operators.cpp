#include "operators.hpp"

#include "TID.hpp"
#include "exceptions.hpp"
#include <memory>
#include <queue>

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

// TODO: split operators, operations and casting into 3 separate files
bool set_up_casting = false;

// LMAO fuck c++ header file system how to create variable in the header file so I don't have to
// define it here and rather in TID.hpp, wtf
const PrimitiveVariableType types[] = {
  PrimitiveVariableType::kInt8, PrimitiveVariableType::kInt16, PrimitiveVariableType::kInt32,
  PrimitiveVariableType::kInt64, PrimitiveVariableType::kUint8, PrimitiveVariableType::kUint16,
  PrimitiveVariableType::kUint32, PrimitiveVariableType::kUint64, PrimitiveVariableType::kF32,
  PrimitiveVariableType::kF64, PrimitiveVariableType::kBool, PrimitiveVariableType::kChar
};
const uint8_t kPrimitiveVariableTypeCount = 12;
std::vector<PrimitiveVariableType> up_cast[kPrimitiveVariableTypeCount];
uint8_t layer[kPrimitiveVariableTypeCount];
// -1 - can't cast
//  0 - can cast without losses
//  1 - can cast but with losses
int8_t can_cast[kPrimitiveVariableTypeCount][kPrimitiveVariableTypeCount];

void SetUpCastingPrimitives() {
  for (uint8_t i = 0; i < kPrimitiveVariableTypeCount; ++i)
    std::fill(can_cast[i], can_cast[i] + kPrimitiveVariableTypeCount, -1);

  std::vector<uint8_t> rg[kPrimitiveVariableTypeCount];
#define Cast(x, y) up_cast[static_cast<uint8_t>(PrimitiveVariableType::x)].push_back(PrimitiveVariableType::y);\
  rg[static_cast<uint8_t>(PrimitiveVariableType::y)].push_back(static_cast<uint8_t>(PrimitiveVariableType::x))

  Cast(kBool, kInt8);
  Cast(kChar, kUint8);

  Cast(kInt8, kUint8);
  Cast(kInt8, kInt16);
  Cast(kUint8, kUint16);

  Cast(kInt16, kUint16);
  Cast(kInt16, kInt32);
  Cast(kUint16, kUint32);

  Cast(kInt32, kUint32);
  Cast(kInt32, kInt64);
  Cast(kUint32, kUint64);
  Cast(kInt32, kF32);
  Cast(kUint32, kF32);
  Cast(kF32, kF64);

  Cast(kInt64, kUint64);
  Cast(kInt64, kF64);
  Cast(kUint64, kF64);

  std::fill(layer, layer + kPrimitiveVariableTypeCount, -1);
  layer[static_cast<uint8_t>(PrimitiveVariableType::kF64)] = 0;
  std::queue<uint8_t> q;
  q.push(static_cast<uint8_t>(PrimitiveVariableType::kF64));
  while (!q.empty()) {
    uint8_t v = q.front();
    q.pop();
    for (uint8_t u : rg[v]) {
      if (layer[u] > layer[v] + 1) {
        layer[u] = layer[v] + 1;
        q.push(u);
      }
    }
  }

  for (uint8_t x = 0; x < kPrimitiveVariableTypeCount; ++x) {
    for (PrimitiveVariableType type : up_cast[x]) {
      can_cast[x][static_cast<uint8_t>(type)] = 0;
      can_cast[x][static_cast<uint8_t>(type)] = 1;
    }
  }
  for (uint8_t through = 0; through < kPrimitiveVariableTypeCount; ++through) {
    for (uint8_t a = 0; a < kPrimitiveVariableTypeCount; ++a) {
      for (uint8_t b = 0; b < kPrimitiveVariableTypeCount; ++b) {
        if (can_cast[a][through] != -1 && can_cast[through][b] != -1) {
          int8_t x = can_cast[a][through] || can_cast[through][b];
          if (can_cast[a][b] == -1 || x < can_cast[a][b])
            can_cast[a][b] = x;
        }
      }
    }
  }

  set_up_casting = true;
}

int8_t CastValue(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  if (from == to) return 0;
  if (from->GetType() != to->GetType()) return -1;
  if (from->GetType() != VariableType::kPrimitive) {
    // we don't cast complex structs and functions, also arrays, but can if we really want
    // however pointers - we do convert these, type punning go weeeeeeeeee
    return from->GetType() == VariableType::kPointer ? 0 : -1;
  }
  // which primitives we can cast to which
  if (!set_up_casting) SetUpCastingPrimitives();
  PrimitiveVariableType from_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(from)->GetPrimitiveType();
  PrimitiveVariableType to_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(to)->GetPrimitiveType();
  return can_cast[static_cast<uint8_t>(from_pr)][static_cast<uint8_t>(to_pr)];
}

bool CanCastLossless(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValue(from, to) == 0;
}

bool CanCast(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValue(from, to) != -1;
}

std::shared_ptr<TIDVariableType> LeastCommonType(const std::shared_ptr<TIDVariableType> & lhs,
    const std::shared_ptr<TIDVariableType> & rhs) {
  if (lhs == rhs) return lhs;
  if (CanCastLossless(lhs, rhs)) return rhs;
  if (CanCastLossless(rhs, lhs)) return lhs;
  if (lhs->GetType() != VariableType::kPrimitive || rhs->GetType() != VariableType::kPrimitive)
    return {};
  PrimitiveVariableType type1 = std::static_pointer_cast<TIDPrimitiveVariableType>(lhs)->GetPrimitiveType();
  PrimitiveVariableType type2 = std::static_pointer_cast<TIDPrimitiveVariableType>(rhs)->GetPrimitiveType();
  uint8_t utype1 = static_cast<uint8_t>(type1);
  uint8_t utype2 = static_cast<uint8_t>(type2);

  int32_t closest = 255;
  PrimitiveVariableType vert = PrimitiveVariableType::kUnknown;
  for (PrimitiveVariableType type : types) {
    auto utype = static_cast<uint8_t>(type);
    auto mid_type = GetPrimitiveVariableType(type);
    if (CanCastLossless(lhs, mid_type) && CanCastLossless(mid_type, rhs)
        && std::max(layer[utype1] - layer[utype], layer[utype2] - layer[utype])) {
      closest = std::max(layer[utype1] - layer[utype], layer[utype2] - layer[utype]);
      vert = type;
    }
  }
  return GetPrimitiveVariableType(vert);
}

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
      BinaryOperator::kAssignment, BinaryOperator::kAdditionAssignment, BinaryOperator::kSubtractionAssignment, BinaryOperator::kMultiplicationAssignment,
      BinaryOperator::kDivisionAssignment, BinaryOperator::kModulusAssignment, BinaryOperator::kBitwiseShiftLeftAssignment,
      BinaryOperator::kBitwiseShiftRightAssignment, BinaryOperator::kBitwiseAndAssignment, BinaryOperator::kBitwiseOr, BinaryOperator::kBitwiseXor
    };
    for (PrimitiveVariableType primitive_type : integer) {
      std::shared_ptr<TIDVariableType> type = SetConstToType(GetPrimitiveVariableType(primitive_type), true);


      std::shared_ptr<TIDVariableType> ref_type = SetReferenceToType(type, true);
    }
    std::shared_ptr<TIDVariableType> bool_type = GetPrimitiveVariableType(PrimitiveVariableType::kBool);
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kInvert)][bool_type] = bool_type;
    unary_operations[static_cast<uint8_t>(UnaryPrefixOperator::kTilda)][bool_type] = bool_type;

    // TODO: add new and delete operators (maybe)
  }

  set_up_binary = true;
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
  if (!set_up_binary) SetUpBinaryOperations();
  // TODO: Figure out how to check both operands casting to non_reference/const
}
