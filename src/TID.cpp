#include "TID.hpp"

#include "exceptions.hpp"
#include <memory>

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

std::shared_ptr<TIDVariableType> TIDComplexVariableType::GetField(std::wstring & name) const {
  for (auto [field_name, value] : contents_)
    if (field_name == name)
      return value;
  return { };
}

std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> origin;
std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> const_non_ref, const_ref, non_const_ref;

std::shared_ptr<TIDVariableType> Copy(const std::shared_ptr<TIDVariableType> & type) {
  std::shared_ptr<TIDVariableType> ans;
  switch (type->GetType()) {
    case VariableType::kArray:
      ans = std::make_shared<TIDArrayVariableType>(*std::static_pointer_cast<TIDArrayVariableType>(type));
      break;
    case VariableType::kComplex:
      ans = std::make_shared<TIDComplexVariableType>(*std::static_pointer_cast<TIDComplexVariableType>(type));
      break;
    case VariableType::kFunction:
      ans = std::make_shared<TIDFunctionVariableType>(*std::static_pointer_cast<TIDFunctionVariableType>(type));
      break;
    case VariableType::kPointer:
      ans = std::make_shared<TIDPointerVariableType>(*std::static_pointer_cast<TIDPointerVariableType>(type));
      break;
    case VariableType::kPrimitive:
      ans = std::make_shared<TIDPrimitiveVariableType>(*std::static_pointer_cast<TIDPrimitiveVariableType>(type));
      break;
  }
  return ans;
}

void CreateIfNecessary(const std::shared_ptr<TIDVariableType> & type) {
  if (const_ref.count(type)) return;
  auto const_ref_type = Copy(type);
  auto const_non_ref_type = Copy(type);
  auto non_const_ref_type = Copy(type);
  const_ref_type->SetConst(true); const_ref_type->SetReference(true);
  const_non_ref_type->SetConst(true);
  non_const_ref_type->SetReference(true);
  origin[type] = type;
  origin[const_ref_type] = type;
  origin[const_non_ref_type] = type;
  origin[non_const_ref_type] = type;
  const_ref[type] = const_ref_type;
  const_non_ref[type] = const_non_ref_type;
  non_const_ref[type] = non_const_ref_type;
}


std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType type) {
  static std::map<PrimitiveVariableType, std::shared_ptr<TIDVariableType>> cache;
  if (!cache.count(type)) {
    cache[type] = std::make_shared<TIDPrimitiveVariableType>(TIDPrimitiveVariableType::Guard(0), type);
    CreateIfNecessary(cache[type]);
  }
  return cache[type];
}


std::shared_ptr<TIDVariableType> DerivePointerFromType(const std::shared_ptr<TIDVariableType> & type) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  if (!type->IsConst() && !type->IsReference()) CreateIfNecessary(type);
  if (!cache.count(type)) {
    cache[type] = std::make_shared<TIDPointerVariableType>(TIDPointerVariableType::Guard(0), origin[type]);
    CreateIfNecessary(cache[type]);
  }
  return SetParamsToType(cache[type], type->IsConst(), type->IsReference());
}

std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  if (!type->IsConst() && !type->IsReference()) CreateIfNecessary(type);
  if (!cache.count(type)) {
    cache[type] = std::make_shared<TIDArrayVariableType>(TIDArrayVariableType::Guard(0), origin[type]);
    CreateIfNecessary(cache[type]);
  }
  return SetParamsToType(cache[type], type->IsConst(), type->IsReference());
}

std::shared_ptr<TIDVariableType> GetTypeWithParameters(const std::shared_ptr<TIDVariableType> & type, bool _const, bool _ref) {
  if (type->IsConst() || type->IsReference())
    return GetTypeWithParameters(origin[type], _const, _ref);
  if (_const) {
    if (_ref)
      return const_ref[type];
    else
      return const_non_ref[type];
  } else {
    if (_ref)
      return non_const_ref[type];
    else
      return type;
  }
}

std::shared_ptr<TIDVariableType> SetConstToType(const std::shared_ptr<TIDVariableType> & type, bool _const) {
  return SetParamsToType(type, _const, type->IsReference());
}

std::shared_ptr<TIDVariableType> SetReferenceToType(const std::shared_ptr<TIDVariableType> & type, bool _ref) {
  return SetParamsToType(type, type->IsConst(), _ref);
}

std::shared_ptr<TIDVariableType> SetParamsToType(const std::shared_ptr<TIDVariableType> & type, bool _const, bool _ref) {
  if (!type->IsConst() && !type->IsReference()) CreateIfNecessary(type);
  return GetTypeWithParameters(type, _const, _ref);
}

std::vector<std::shared_ptr<TIDVariableType>> GetDerivedTypes(const std::shared_ptr<TIDVariableType> & type) {
  std::vector<std::shared_ptr<TIDVariableType>> result;
  result.push_back(type);
  if (type->IsReference())
    result.push_back(SetReferenceToType(type, false));
  if (!type->IsConst())
    result.push_back(SetConstToType(type, true));
  if (type->IsReference() && !type->IsConst())
    result.push_back(SetParamsToType(type, true, false));
  return result;
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
  if (!complex_struct || complex_struct->GetType() != VariableType::kComplex)
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
