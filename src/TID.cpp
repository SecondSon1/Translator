#include "TID.hpp"

#include "exceptions.hpp"
#include "operators.hpp"
#include <memory>
#include <string>
#include "generation.hpp"

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

std::wstring names[kPrimitiveVariableTypeCount];
std::map<std::wstring, PrimitiveVariableType> dict;
void SetUpNames() {
  static bool set_up = false;
  if (set_up) return;
#define Set(key, value) names[static_cast<uint8_t>(PrimitiveVariableType::key)] = value; \
  dict[value] = PrimitiveVariableType::key
  Set(kInt8, L"int8");
  Set(kInt16, L"int16");
  Set(kInt32, L"int32");
  Set(kInt64, L"int64");
  Set(kUint8, L"uint8");
  Set(kUint16, L"uint16");
  Set(kUint32, L"uint32");
  Set(kUint64, L"uint64");
  Set(kChar, L"char");
  Set(kBool, L"bool");
  Set(kF32, L"f32");
  Set(kF64, L"f64");
#undef Set
  set_up = true;
}

std::wstring ToString(PrimitiveVariableType type) {
  SetUpNames();
  return names[static_cast<uint8_t>(type)];
}

PrimitiveVariableType FromWstringToPrimitiveType(const std::wstring & str) {
  SetUpNames();
  if (dict.count(str)) return dict[str];
  else return PrimitiveVariableType::kUnknown;
}

std::shared_ptr<TIDVariableType> TIDComplexVariableType::GetField(std::wstring & name) const {
  for (auto [field_name, value] : contents_)
    if (field_name == name)
      return value;
  return { };
}
uint32_t TIDComplexVariableType::GetOffset(std::wstring & name) const {
  uint32_t offset = 0;
  for (auto [field_name, value] : contents_) {
    if (field_name == name)
      return offset;
    offset += value->GetSize();
  }
  return offset;
}

std::wstring TIDPrimitiveVariableType::ToString() const {
  std::wstring result;
  if (IsConst()) result += L"const ";
  result += ::ToString(type_);
  if (IsReference()) result += L" &";
  return result;
}

std::wstring TIDComplexVariableType::ToString() const {
  std::wstring result;
  if (IsConst()) result += L"const ";
  result += name_;
  if (IsReference()) result += L" &";
  return result;
}

std::wstring TIDFunctionVariableType::ToString() const {
  std::wstring result;
  if (IsConst()) result += L"const ";
  result += L"function<" + (return_type_ ? return_type_->ToString() : L"void") + L"(";
  bool was_first = false;
  if (!parameters_.empty()) {
    result += parameters_[0]->ToString();
    was_first = true;
    for (size_t i = 1; i < parameters_.size(); ++i)
      result += L", " + parameters_[i]->ToString();
  }
  if (!default_parameters_.empty()) {
    if (!was_first)
      result += default_parameters_[0]->ToString() + L"=";
    for (size_t i = !was_first; i < default_parameters_.size(); ++i)
      result += L", " + default_parameters_[i]->ToString() + L"=";
  }
  result += L")>";
  if (IsReference()) result += L" &";
  return result;
}

std::wstring TIDPointerVariableType::ToString() const {
  std::wstring result;
  if (IsConst()) result += L"const ";
  if (value_->GetType() == VariableType::kArray)
    result += L"*(" + value_->ToString() + L")";
  else
    result += L"*" + value_->ToString();
  if (IsReference()) result += L" &";
  return result;
}

std::wstring TIDArrayVariableType::ToString() const {
  std::wstring result;
  if (IsConst()) result += L"const ";
  result += value_->ToString() + L"[]";
  if (IsReference()) result += L" &";
  return result;
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
  SetReferenceToType(type, false);
  if (!cache.count(type)) {
    cache[type] = std::make_shared<TIDPointerVariableType>(TIDPointerVariableType::Guard(0), origin[type]);
    CreateIfNecessary(cache[type]);
  }
  return cache[type];
}

std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type) {
  static std::map<std::shared_ptr<TIDVariableType>, std::shared_ptr<TIDVariableType>> cache;
  SetReferenceToType(type, false);
  if (!cache.count(type)) {
    cache[type] = std::make_shared<TIDArrayVariableType>(TIDArrayVariableType::Guard(0), origin[type]);
    CreateIfNecessary(cache[type]);
  }
  return cache[type];
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

std::vector<std::shared_ptr<TIDValue>> GetDerivedTypes(const std::shared_ptr<TIDValue> & val) {
  std::vector<std::shared_ptr<TIDValue>> result;
  auto type = val->GetType();
  result.push_back(val);
  if (type->IsReference())
    result.push_back(std::make_shared<TIDTemporaryValue>(SetReferenceToType(type, false)));
  if (!type->IsConst())
    result.push_back(std::make_shared<TIDTemporaryValue>(SetConstToType(type, true)));
  if (type->IsReference() && !type->IsConst())
    result.push_back(std::make_shared<TIDTemporaryValue>(SetParamsToType(type, true, false)));
  return result;
}

void TID::LoadVariableAddress(const std::wstring & name, RPN & rpn) const {
  for (size_t i = nodes_.size() - 1; ~i; --i) {
    if (nodes_[i].variables_.count(name)) {
      rpn.PushNode(RPNOperand(nodes_[i].variables_.at(name)->GetAddress()));
      if (nodes_[i].func_name_ == nodes_.back().func_name_) {
        rpn.PushNode(RPNOperator(RPNOperatorType::kFromSP));
      } else {
        rpn.PushNode(RPNReferenceOperand(nodes_[i].func_name_));
        rpn.PushNode(RPNOperator(RPNOperatorType::kFuncSP));
        rpn.PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
      }
      return;
    }
  }
  assert(false);
}

void TID::AddScope() {
  nodes_.emplace_back();
  nodes_.back().next_address_ = nodes_[nodes_.size() - 2].next_address_;
  nodes_.back().func_name_ = nodes_[nodes_.size() - 2].func_name_;
}

void TID::AddFunctionScope(const std::wstring & name, const std::shared_ptr<TIDVariableType> & return_type) {
  nodes_.emplace_back();
  nodes_.back().func_name_ = name;
  uint32_t size_for_return = return_type ? return_type->GetSize() + 1 : 0;
  nodes_.back().next_address_ = size_for_return + 8 /* first 8 bytes are pointer for where to return after function is complete */;
  max_address_.push_back(nodes_.back().next_address_);
}

void TID::RemoveScope() {
  if (nodes_.size() == 1)
    throw NoScopeAvailableError();
  nodes_.pop_back();
  nodes_.back().child_node_cnt_++;
}

void TID::RemoveFunctionScope() {
  RemoveScope();
  max_address_.pop_back();
}

void TID::AddComplexStruct(const Lexeme & lexeme, const std::shared_ptr<TIDVariableType> & complex_struct) {
  if (!complex_struct || complex_struct->GetType() != VariableType::kComplex)
    throw NotComplexStructError();
  std::wstring name = std::static_pointer_cast<TIDComplexVariableType>(complex_struct)->GetName();
  if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
    throw ConflictingNames(lexeme);
  std::dynamic_pointer_cast<TIDComplexVariableType>(complex_struct)
    ->SetInternalPrefix(GetCurrentPrefix());
  nodes_.back().complex_structs_[name] = complex_struct;
}

void TID::AddVariable(const Lexeme & lexeme, const std::wstring & name,
    const std::shared_ptr<TIDVariableType> & type) {
  if (!type)
    throw VoidNotExpected(lexeme);
  if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
    throw ConflictingNames(lexeme);
  std::shared_ptr<TIDVariable> var = std::make_shared<TIDVariable>(name, GetCurrentPrefix(), type,
      nodes_.back().next_address_);
  nodes_.back().next_address_ += type->GetSize();
  max_address_.back() = std::max(max_address_.back(), nodes_.back().next_address_);
  nodes_.back().variables_[name] = var;
}
uint64_t TID::AddTemporaryInstance([[maybe_unused]] const Lexeme & lexeme,
                                     const std::shared_ptr<TIDVariableType> & type) {
  if (!type)
    throw VoidNotExpected(lexeme);
  std::wstring name = L"$" + std::to_wstring(temp_structs++);
  std::shared_ptr<TIDVariable> var = std::make_shared<TIDVariable>(name, GetCurrentPrefix(), type,
      nodes_.back().next_address_);
  nodes_.back().next_address_ += type->GetSize();
  max_address_.back() = std::max(max_address_.back(), nodes_.back().next_address_);
  return nodes_.back().next_address_ - type->GetSize();
}

std::wstring TID::GetCurrentPrefix() const {
  std::wstring ans;
  for (const TIDNode & node : nodes_)
    ans += std::to_wstring(node.child_node_cnt_) + L"::";
  return ans;
}
