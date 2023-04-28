#pragma once

#include <map>
#include <string>
#include "TID.hpp"

constexpr uint8_t kOperatorCount = 44;
constexpr uint8_t kUnaryPrefixOperatorCount = 10;
constexpr uint8_t kUnaryPostfixOperatorCount = 2;
constexpr uint8_t kBinaryOperatorCount = 32;

enum class OperatorType : uint8_t {
  kUnaryPrefix, kUnaryPostfix, kBinary
};

enum class UnaryPrefixOperator : uint8_t {
  kIncrement                    = 0,
  kDecrement                    = 1,
  kPlus                         = 2,
  kMinus                        = 3,
  kInvert                       = 4,
  kTilda                        = 5,
  kDereference                  = 6,
  kAddressOf                    = 7,
  kNew                          = 8,
  kDelete                       = 9,
  kUnknown                      = 253
};

enum class UnaryPostfixOperator : uint8_t {
  kIncrement                    = 10,
  kDecrement                    = 11,
  kUnknown                      = 254
};

enum class BinaryOperator : uint8_t {
  kFunctionCall                 = 12,
  kSubscript                    = 13,
  kMemberAccess                 = 14,

  kMultiplication               = 15,
  kDivision                     = 16,
  kModulus                      = 17,
  kAddition                     = 18,
  kSubtraction                  = 19,
  kBitwiseShiftLeft             = 20,
  kBitwiseShiftRight            = 21,
  kLess                         = 22,
  kMore                         = 23,
  kLessOrEqual                  = 24,
  kMoreOrEqual                  = 25,
  kEqual                        = 26,
  kNotEqual                     = 27,
  kBitwiseAnd                   = 28,
  kBitwiseXor                   = 29,
  kBitwiseOr                    = 30,
  kLogicalAnd                   = 31,
  kLogicalOr                    = 32,
  kAssignment                   = 33,
  kAdditionAssignment           = 34,
  kSubtractionAssignment        = 35,
  kMultiplicationAssignment     = 36,
  kDivisionAssignment           = 37,
  kModulusAssignment            = 38,
  kBitwiseShiftLeftAssignment   = 39,
  kBitwiseShiftRightAssignment  = 40,
  kBitwiseAndAssignment         = 41,
  kBitwiseOrAssignment          = 42,
  kBitwiseXorAssignment         = 43,
  kUnknown                      = 255
};

class Operator {
 public:
  Operator(UnaryPrefixOperator op) : type_(OperatorType::kUnaryPrefix), u_pre_op_(op) {}
  Operator(UnaryPostfixOperator op) : type_(OperatorType::kUnaryPostfix), u_post_op_(op) {}
  Operator(BinaryOperator op) : type_(OperatorType::kBinary), bin_op_(op) {}

  OperatorType GetOperatorType() const { return type_; }
  UnaryPrefixOperator GetUnaryPrefixOperator() const;
  UnaryPostfixOperator GetUnaryPostfixOperator() const;
  BinaryOperator GetBinaryOperator() const;

  std::wstring ToString() const;

 private:
  OperatorType type_;
  union {
    UnaryPrefixOperator u_pre_op_;
    UnaryPostfixOperator u_post_op_;
    BinaryOperator bin_op_;
  };
};

std::wstring ToString(UnaryPrefixOperator op);
std::wstring ToString(UnaryPostfixOperator op);
std::wstring ToString(BinaryOperator op);

UnaryPrefixOperator UnaryPrefixOperatorByString(const std::wstring & str);
UnaryPostfixOperator UnaryPostfixOperatorByString(const std::wstring & str);
BinaryOperator BinaryOperatorByString(const std::wstring & str);
