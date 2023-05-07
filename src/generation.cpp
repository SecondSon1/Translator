#include "generation.hpp"
#include "TID.hpp"
#include <memory>

std::wstring ToString(RPNOperatorType type) {
#define OperatorCase(x) case RPNOperatorType::x:\
  return L"" #x
  switch (type) {
    OperatorCase(kLoad);
    OperatorCase(kStore);
    OperatorCase(kJmp);
    OperatorCase(kCall);
    OperatorCase(kJz);
    OperatorCase(kPush);
    OperatorCase(kPop);
    OperatorCase(kSP);
    OperatorCase(kFromSP);
    OperatorCase(kNew);
    OperatorCase(kDelete);
    OperatorCase(kReturn);
    OperatorCase(kFuncSP);
    OperatorCase(kDump);
    OperatorCase(kF64to32);
    OperatorCase(kF32to64);
    OperatorCase(kToF64);
    OperatorCase(kFromF64);
    OperatorCase(kToBool);
    OperatorCase(kMinus);
    OperatorCase(kInvert);
    OperatorCase(kTilda);
    OperatorCase(kAdd);
    OperatorCase(kSubtract);
    OperatorCase(kMultiply);
    OperatorCase(kDivide);
    OperatorCase(kModulus);
    OperatorCase(kBitwiseShiftLeft);
    OperatorCase(kBitwiseShiftRight);
    OperatorCase(kBitwiseAnd);
    OperatorCase(kBitwiseOr);
    OperatorCase(kBitwiseXor);
    OperatorCase(kLess);
    OperatorCase(kMore);
    OperatorCase(kLessOrEqual);
    OperatorCase(kMoreOrEqual);
    OperatorCase(kEqual);
    OperatorCase(kNotEqual);
  }
#undef OperatorCase
}

PrimitiveVariableType GetTypeOfVariable(const std::shared_ptr<TIDVariableType> & type) {
  if (!type || type->GetType() == VariableType::kComplex)
    return PrimitiveVariableType::kUnknown;
  return type->GetType() != VariableType::kPrimitive ? PrimitiveVariableType::kUint64 :
    std::dynamic_pointer_cast<TIDPrimitiveVariableType>(type)->GetPrimitiveType();
}
