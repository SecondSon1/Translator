#include "TID.hpp"

#include "exceptions.hpp"

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
    case PrimitiveVariableType::kUnknown:
      return 0;
  }
}

PrimitiveVariableType FromWstringToPrimitiveType(const std::wstring & str) {
  static bool set_up = false;
  static std::map<std::wstring, PrimitiveVariableType> dict;
  if (!set_up) {
    dict[L"int8"] = PrimitiveVariableType::kInt8;
    dict[L"int16"] = PrimitiveVariableType::kInt16;
    dict[L"int32"] = PrimitiveVariableType::kInt32;
    dict[L"int64"] = PrimitiveVariableType::kInt64;
    dict[L"uint8"] = PrimitiveVariableType::kUint8;
    dict[L"uint16"] = PrimitiveVariableType::kUint16;
    dict[L"uint32"] = PrimitiveVariableType::kUint32;
    dict[L"uint64"] = PrimitiveVariableType::kUint64;
    dict[L"char"] = PrimitiveVariableType::kChar;
    dict[L"bool"] = PrimitiveVariableType::kBool;
    dict[L"f32"] = PrimitiveVariableType::kF32;
    dict[L"f64"] = PrimitiveVariableType::kF32;
    set_up = true;
  }
  if (dict.count(str)) return dict[str];
  else return PrimitiveVariableType::kUnknown;
}

std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType type) {
  static std::map<PrimitiveVariableType, std::shared_ptr<TIDVariableType>> cache;
  if (!cache.count(type))
    cache[type] = std::make_shared<TIDPrimitiveVariableType>(TIDPrimitiveVariableType::Guard(0), type);
  return cache[type];
}

std::shared_ptr<TIDVariableType> DerivePointerFromType(const std::shared_ptr<TIDVariableType> & type) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  if (!cache.count(type))
    cache[type] = std::make_shared<TIDPointerVariableType>(TIDPointerVariableType::Guard(0), type);
  return cache[type];
}

std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  if (!cache.count(type))
    cache[type] = std::make_shared<TIDArrayVariableType>(TIDArrayVariableType::Guard(0), type);
  return cache[type];
}

std::shared_ptr<TIDVariableType> Copy(const std::shared_ptr<TIDVariableType> & type) {
  std::shared_ptr<TIDVariableType> ans;
  switch (type->GetType()) {
    case VariableType::kArray:
      ans = std::make_shared<TIDArrayVariableType>(*std::static_pointer_cast<TIDArrayVariableType>(type));
      break;
    case VariableType::kComplex:
      ans = std::make_shared<TIDComplexVariableType>(*type);
      break;
    case VariableType::kFunction:
      ans = std::make_shared<TIDFunctionVariableType>(*type);
      break;
    case VariableType::kPointer:
      ans = std::make_shared<TIDPointerVariableType>(*type);
      break;
    case VariableType::kPrimitive:
      ans = std::make_shared<TIDPrimitiveVariableType>(*type);
      break;
  }
  return ans;
}

std::shared_ptr<TIDVariableType> SetConstToType(const std::shared_ptr<TIDVariableType> & type, bool _const) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  if (type->IsConst() == _const) return type;
  if (!cache.count(type)) {
    std::shared_ptr<TIDVariableType> ptr = Copy(type);
    ptr->SetConst(_const);
    cache[type] = std::move(ptr);
  }
  return cache[type];
}

std::shared_ptr<TIDVariableType> SetReferenceToType(const std::shared_ptr<TIDVariableType> & type, bool _ref) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  if (type->IsConst() == _ref) return type;
  if (!cache.count(type)) {
    std::shared_ptr<TIDVariableType> ptr = Copy(type);
    ptr->SetReference(_ref);
    cache[type] = std::move(ptr);
  }
  return cache[type];
}

void TID::AddScope() {
  nodes_.emplace_back();
}

void TID::RemoveScope() {
  if (nodes_.size() == 1)
    throw NoScopeAvailableError();
  nodes_.pop_back();
}
void TID::AddComplexStruct(const Lexeme & lexeme, const std::wstring & name,
    const std::shared_ptr<TIDVariableType> & complex_struct) {
  if (std::dynamic_pointer_cast<TIDComplexVariableType>(complex_struct) == nullptr)
    throw NotComplexStructError();
  if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
    throw ConflictingNames(lexeme);
  nodes_.back().complex_structs_[name] = complex_struct;
}

void TID::AddVariable(const Lexeme & lexeme, const std::wstring & name,
    const std::shared_ptr<TIDVariableType> & variable) {
  if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
    throw ConflictingNames(lexeme);
  nodes_.back().variables_[name] = variable;
}
