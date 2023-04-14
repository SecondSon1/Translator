#include "TID.hpp"


uint32_t GetSizeOfPrimitive(const PrimitiveVariableType & type) {
  switch (type) {
    case PrimitiveVariableType::kInt8:
    case PrimitiveVariableType::kUint8:
    case PrimitiveVariableType::kChar:
    case PrimitiveVariableType::kBool:
      return 1;
    case PrimitiveVariableType::kInt16:
    case PrimitiveVariableType::kUint16:
      return 2;
    case PrimitiveVariableType::kInt32:
    case PrimitiveVariableType::kUint32:
    case PrimitiveVariableType::kF32:
      return 4;
    case PrimitiveVariableType::kInt64:
    case PrimitiveVariableType::kUint64:
    case PrimitiveVariableType::kF64:
      return 8;
  }
}
