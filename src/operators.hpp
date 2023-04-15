#pragma once

#include <map>
#include <string>

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
  kFunctionCall                 = 12,
  kSubscript                    = 13,
  kUnknown                      = 254
};

enum class BinaryOperator : uint8_t {
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

std::wstring ToString(UnaryPrefixOperator op);
std::wstring ToString(UnaryPostfixOperator op);
std::wstring ToString(BinaryOperator op);

UnaryPrefixOperator UnaryPrefixOperatorByString(const std::wstring & str);
UnaryPostfixOperator UnaryPostfixOperatorByString(const std::wstring & str);
BinaryOperator BinaryOperatorByString(const std::wstring & str);
