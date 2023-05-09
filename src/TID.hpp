#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include "lexeme.hpp"

enum class VariableType : uint8_t {
  kPrimitive, kComplex, kFunction, kPointer, kArray
};

class TIDVariableType {
 public:
  virtual ~TIDVariableType() = default;
  VariableType GetType() const { return variable_type_; }
  uint32_t GetSize() const { return ref_ ? 8 : size_; } // reference is implicit pointer

  bool IsConst() const { return const_; }
  bool IsReference() const { return ref_; }
  void SetConst(bool val) { const_ = val; }
  void SetReference(bool val) { ref_ = val; }

  virtual std::wstring ToString() const = 0;

 protected:
  TIDVariableType(VariableType type, uint32_t size)
      : variable_type_(type), size_(size), const_(false), ref_(false) {}
  void SetSize(uint32_t size) { size_ = size; }

 private:
  VariableType variable_type_;
  uint32_t size_;
  bool const_, ref_;
};

// if you change this list you need to change another few
// parameters in operators.cpp in casting section
constexpr uint8_t kPrimitiveVariableTypeCount = 13;
enum class PrimitiveVariableType : uint8_t {
  kInt8 = 0, kInt16 = 1, kInt32 = 2, kInt64 = 3,
  kUint8 = 4, kUint16 = 5, kUint32 = 6, kUint64 = 7,
  kF32 = 8, kF64 = 9, kBool = 10, kChar = 11, kUnknown = 12
};
constexpr PrimitiveVariableType types[] = {
  PrimitiveVariableType::kInt8, PrimitiveVariableType::kInt16, PrimitiveVariableType::kInt32,
  PrimitiveVariableType::kInt64, PrimitiveVariableType::kUint8, PrimitiveVariableType::kUint16,
  PrimitiveVariableType::kUint32, PrimitiveVariableType::kUint64, PrimitiveVariableType::kF32,
  PrimitiveVariableType::kF64, PrimitiveVariableType::kBool, PrimitiveVariableType::kChar
};

uint32_t GetSizeOfPrimitive(const PrimitiveVariableType & type);
std::wstring ToString(PrimitiveVariableType type);
PrimitiveVariableType FromWstringToPrimitiveType(const std::wstring & str);

class TIDPrimitiveVariableType : public TIDVariableType {
 private:
  struct Guard { explicit Guard(int) {} };

 public:
  TIDPrimitiveVariableType([[maybe_unused]] Guard _, PrimitiveVariableType & type)
    : TIDVariableType(VariableType::kPrimitive, GetSizeOfPrimitive(type)), type_(type) {}

  PrimitiveVariableType GetPrimitiveType() const { return type_; }

  std::wstring ToString() const override;

 private:
  friend std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType);

 private:
  PrimitiveVariableType type_;
};

class TIDComplexVariableType : public TIDVariableType {
 public:
  TIDComplexVariableType(const std::wstring & name, const std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> & contents)
    : TIDVariableType(VariableType::kComplex, 0), name_(name), contents_(contents) {
    uint32_t our_size = 0;
    for (const auto & [field_name, type] : contents_)
      our_size += type->GetSize();
    SetSize(our_size);
  }

  const std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> & GetContents() const { return contents_; }

  std::wstring GetName() const { return name_; }
  std::wstring GetInternalName() const { return internal_name_; }
  std::shared_ptr<TIDVariableType> GetField(std::wstring & name) const;
  uint32_t GetOffset(std::wstring & name) const;

  std::wstring ToString() const override;

 private:
  friend class TID;
  void SetInternalPrefix(const std::wstring & prefix) {
    internal_name_ = prefix + name_;
  }

 private:
  std::wstring name_;
  std::wstring internal_name_;
  std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> contents_;
};

class TIDFunctionVariableType : public TIDVariableType {
 public:
  TIDFunctionVariableType(const std::shared_ptr<TIDVariableType> & return_type,
      const std::vector<std::shared_ptr<TIDVariableType>> & parameters,
      const std::vector<std::shared_ptr<TIDVariableType>> & default_parameters)
    : TIDVariableType(VariableType::kFunction, 8), return_type_(return_type),
      parameters_(parameters), default_parameters_(default_parameters) {}

  std::shared_ptr<TIDVariableType> GetReturnType() const { return return_type_; }
  const std::vector<std::shared_ptr<TIDVariableType>> & GetParameters() const { return parameters_; }
  const std::vector<std::shared_ptr<TIDVariableType>> & GetDefaultParameters() const { return default_parameters_; }

  std::wstring ToString() const override;

 private:
  std::shared_ptr<TIDVariableType> return_type_;
  std::vector<std::shared_ptr<TIDVariableType>> parameters_, default_parameters_;
};

class TIDPointerVariableType : public TIDVariableType {
 private:
  struct Guard { explicit Guard(int) {} };

 public:
  TIDPointerVariableType([[maybe_unused]] Guard _, const std::shared_ptr<TIDVariableType> & pointing_to)
    : TIDVariableType(VariableType::kPointer, 8), value_(pointing_to) {}
  std::shared_ptr<TIDVariableType> GetValue() const { return value_; }

  std::wstring ToString() const override;

 private:
  friend std::shared_ptr<TIDVariableType> DerivePointerFromType(const std::shared_ptr<TIDVariableType> & type);

 private:
  std::shared_ptr<TIDVariableType> value_;
};

class TIDArrayVariableType : public TIDVariableType {
  private:
  struct Guard { explicit Guard(int) {} };

  public:
  TIDArrayVariableType([[maybe_unused]] Guard _, const std::shared_ptr<TIDVariableType> & value)
      : TIDVariableType(VariableType::kArray, 8), value_(value) {}
  std::shared_ptr<TIDVariableType> GetValue() const { return value_; }

  std::wstring ToString() const override;

  private:
  friend std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type);

  private:
  std::shared_ptr<TIDVariableType> value_;
};

std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType type);
std::shared_ptr<TIDVariableType> DerivePointerFromType(const std::shared_ptr<TIDVariableType> & type);
std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type);

std::shared_ptr<TIDVariableType> SetParamsToType(const std::shared_ptr<TIDVariableType> & type, bool _const, bool _ref);
std::shared_ptr<TIDVariableType> SetConstToType(const std::shared_ptr<TIDVariableType> & type, bool _const);
std::shared_ptr<TIDVariableType> SetReferenceToType(const std::shared_ptr<TIDVariableType> & type, bool _ref);


enum class TIDValueType : uint8_t { // basically rvalue and lvalue (respectively)
  kTemporary, kVariable
};

class TIDValue {
 public:
  virtual ~TIDValue() = default;

  TIDValueType GetValueType() const { return value_type_; }
  std::shared_ptr<TIDVariableType> GetType() const { return type_; }

  virtual std::wstring ToString() const = 0;

 protected:
  TIDValue(TIDValueType value_type, const std::shared_ptr<TIDVariableType> & type)
          : value_type_(value_type), type_(type) {}

 private:
  TIDValueType value_type_;
  std::shared_ptr<TIDVariableType> type_;
};

class TIDTemporaryValue : public TIDValue {
 public:
  TIDTemporaryValue(const std::shared_ptr<TIDVariableType> & type) : TIDValue(TIDValueType::kTemporary, type) {}

  std::wstring ToString() const override {
    auto type = GetType();
    return type ? type->ToString() : L"void";
  }
};

class TIDVariable : public TIDValue {
 public:
  TIDVariable(const std::wstring & name, const std::wstring & internal_prefix, const std::shared_ptr<TIDVariableType> & type, uint32_t address)
             : TIDValue(TIDValueType::kVariable, type), name_(name), internal_name_(internal_prefix + name), address_(address) {}

  std::wstring GetName() const { return name_; }
  std::wstring GetInternalName() const { return internal_name_; }
  uint64_t GetAddress() const { return address_; }

  std::wstring ToString() const override {
    auto type = GetType();
    return name_ + L": " + (type ? type->ToString() : L"void");
  }

 private:
  std::wstring name_;
  std::wstring internal_name_;
  uint64_t address_;
};

std::vector<std::shared_ptr<TIDValue>> GetDerivedTypes(const std::shared_ptr<TIDValue> & type);

class TID {
 public:
  TID() {
    AddFunctionScope(SetConstToType(
      GetPrimitiveVariableType(PrimitiveVariableType::kInt32), true
    ));
  }

  // All of them will return nullptr in std::shared_ptr if complex struct/variable is not found
  std::shared_ptr<const TIDVariableType> GetComplexStruct(const std::wstring & name) const {
    for (size_t scope_index = nodes_.size() - 1; ~scope_index; --scope_index)
      if (nodes_[scope_index].complex_structs_.count(name))
        return nodes_[scope_index].complex_structs_.at(name);
    // Not found, returning empty pointer
    return {};
  }
  std::shared_ptr<TIDVariableType> GetComplexStruct(const std::wstring & name) {
    return std::const_pointer_cast<TIDVariableType>(const_cast<const TID &>(*this).GetComplexStruct(name));
  }

  std::shared_ptr<const TIDVariable> GetVariable(const std::wstring & name) const {
    for (size_t scope_index = nodes_.size() - 1; ~scope_index; --scope_index)
      if (nodes_[scope_index].variables_.count(name))
        return nodes_[scope_index].variables_.at(name);
    // Not found, returning empty variable
    return {};
  }
  std::shared_ptr<TIDVariable> GetVariable(const std::wstring & name) {
    return std::const_pointer_cast<TIDVariable>(const_cast<const TID &>(*this).GetVariable(name));
  }

  std::shared_ptr<const TIDVariable> GetVariableFromCurrentScope(const std::wstring & name) const {
    const auto & vars = nodes_.back().variables_;
    return vars.count(name) ? vars.at(name) : nullptr;
  }
  std::shared_ptr<TIDVariable> GetVariableFromCurrentScope(const std::wstring & name) {
    return std::const_pointer_cast<TIDVariable>(const_cast<const TID &>(*this).GetVariableFromCurrentScope(name));
  }

  uint64_t GetFunctionScopeMaxAddress() const { return max_address_.back(); }

  std::vector<std::shared_ptr<TIDVariable>> GetLastScopeVariables() const {
    std::vector<std::shared_ptr<TIDVariable>> res;
    for (auto [name, var] : nodes_.back().variables_)
      res.push_back(var);
    return res;
  }

  uint64_t GetNextAddress() const { return nodes_.back().next_address_; }

 public:
  void AddScope();
  void AddFunctionScope(const std::shared_ptr<TIDVariableType> & return_type);
  void RemoveScope();
  void RemoveFunctionScope();
  void AddComplexStruct(const Lexeme & lexeme,
                        const std::shared_ptr<TIDVariableType> & complex_struct);
  void AddVariable(const Lexeme & lexeme, const std::wstring & name,
                   const std::shared_ptr<TIDVariableType> & type);
  uint64_t AddTemporaryInstance(const Lexeme & lexeme,
                                const std::shared_ptr<TIDVariableType> & type);

 private:
  std::wstring GetCurrentPrefix() const;

 private:
  struct TIDNode {
    TIDNode() {}
    std::map<std::wstring, std::shared_ptr<TIDVariableType>> complex_structs_;
    std::map<std::wstring, std::shared_ptr<TIDVariable>> variables_;
    uint32_t child_node_cnt_ = 0;
    uint64_t next_address_ = 0;
  };

  uint32_t temp_structs = 0;
  std::vector<TIDNode> nodes_;
  std::vector<uint64_t> max_address_;
};
