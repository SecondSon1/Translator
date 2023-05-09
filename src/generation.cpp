#include "generation.hpp"
#include "TID.hpp"
#include <memory>

std::wstring ToString(RPNOperatorType type) {
#define OperatorCase(x) case RPNOperatorType::x:\
  return L"" #x
  switch (type) {
    OperatorCase(kLoad);
    OperatorCase(kStoreDA);
    OperatorCase(kStoreAD);
    OperatorCase(kJmp);
    OperatorCase(kCall);
    OperatorCase(kJz);
    OperatorCase(kPush);
    OperatorCase(kPop);
    OperatorCase(kSP);
    OperatorCase(kFromSP);
    OperatorCase(kNew);
    OperatorCase(kDelete);
    OperatorCase(kRead);
    OperatorCase(kWrite);
    OperatorCase(kReturn);
    OperatorCase(kFuncSP);
    OperatorCase(kDump);
    OperatorCase(kDuplicate);
    OperatorCase(kSave);
    OperatorCase(kRestore);
    OperatorCase(kCopyFT);
    OperatorCase(kCopyTF);
    OperatorCase(kFill);
    OperatorCase(kToF64);
    OperatorCase(kFromF64);
    OperatorCase(kToBool);
    OperatorCase(kToInt64);
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

void AddReturn(RPN & rpn) {
  auto node = rpn.GetNodes().back();
  if (node->GetNodeType() == NodeType::kOperator &&
      std::dynamic_pointer_cast<RPNOperator>(node)->GetOperatorType() == RPNOperatorType::kReturn)
    return;
  rpn.PushNode(RPNOperator(RPNOperatorType::kReturn));
}

void LoadIfReference(const std::shared_ptr<TIDValue> & val, RPN & rpn) {
  if (IsReference(val))
    rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, GetTypeOfVariable(val->GetType())));
}
void LoadIfReference(const std::shared_ptr<TIDVariableType> & val, RPN & rpn) {
  if (IsReference(val))
    rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, GetTypeOfVariable(val)));
}

void LoadValue(const std::shared_ptr<TIDValue> & val, const std::shared_ptr<TIDVariableType> & type, RPN & rpn) {
  if (IsReference(val))
    rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, GetTypeOfVariable(val->GetType())));
  if (!IsReference(type))
    rpn.PushNode(RPNOperator(RPNOperatorType::kLoad, GetTypeOfVariable(type)));
}

void LoadPointer(const std::shared_ptr<TIDValue> & ptr, RPN & rpn) {
  LoadIfReference(ptr, rpn);
}

bool IsReference(const std::shared_ptr<TIDValue> & value) {
  return value->GetValueType() == TIDValueType::kVariable || value->GetType()->IsReference();
}
bool IsReference(const std::shared_ptr<TIDVariableType> & type) {
  return type->IsReference();
}
