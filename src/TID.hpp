#pragma once

#include "exceptions.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

enum class VariableType : uint8_t {
  kPrimitive, kComplex, kFunction, kPointer, kArray
};

class TIDVariableType {
 public:
  virtual ~TIDVariableType() = default;
  VariableType GetType() const { return variable_type_; }
  uint32_t GetSize() const { return size_; }

 protected:
  TIDVariableType(VariableType type, uint32_t size)
      : variable_type_(type), size_(size) {}
  void SetSize(uint32_t size) { size_ = size; }

 private:
  VariableType variable_type_;
  uint32_t size_;
};

enum class PrimitiveVariableType : uint8_t {
  kInt8 = 0, kInt16 = 1, kInt32 = 2, kInt64 = 3,
  kUint8 = 4, kUint16 = 5, kUint32 = 6, kUint64 = 7,
  kF32 = 8, kF64 = 9, kBool = 10, kChar = 11, kUnknown = 12
};

uint32_t GetSizeOfPrimitive(const PrimitiveVariableType & type);

PrimitiveVariableType FromWstringToPrimitiveType(const std::wstring & str);

class TIDPrimitiveVariableType : public TIDVariableType {
 private:
  struct Guard { explicit Guard(int) {} };

 public:
  TIDPrimitiveVariableType([[maybe_unused]] Guard _, PrimitiveVariableType & type)
    : TIDVariableType(VariableType::kPrimitive, GetSizeOfPrimitive(type)), type_(type) {}

 private:
  friend std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType);

 private:
  PrimitiveVariableType type_;
};

class TIDComplexVariableType : public TIDVariableType {
 public:
  TIDComplexVariableType(const std::vector<std::shared_ptr<TIDVariableType>> & contents)
    : TIDVariableType(VariableType::kComplex, 0), contents_(contents) {
    uint32_t our_size = 0;
    for (const std::shared_ptr<TIDVariableType> & type : contents_)
      our_size += type->GetSize();
    SetSize(our_size);
  }

  const std::vector<std::shared_ptr<TIDVariableType>> & GetContents() const { return contents_; }

 private:
  std::vector<std::shared_ptr<TIDVariableType>> contents_;
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
    : TIDVariableType(VariableType::kPointer, 8), value_(value) {}
  std::shared_ptr<TIDVariableType> GetValue() const { return value_; }

 private:
  friend std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type);

 private:
  std::shared_ptr<TIDVariableType> value_;
};

class TIDVariable {
 public:
  TIDVariable() : type_(nullptr), const_(false), reference_(false) {}
  TIDVariable(const std::shared_ptr<const TIDVariableType> & type) : type_(type), const_(false), reference_(false) {}
  TIDVariable(const std::shared_ptr<const TIDVariableType> & type, bool constant, bool ref)
    : type_(type), const_(constant), reference_(ref) {}

  bool IsEmpty() const { return type_ == nullptr; }

  bool IsConst() const { return const_; }
  bool IsReference() const { return reference_; }

  void SetConst(bool value) { const_ = value; }
  void SetReference(bool value) { reference_ = value; }

  std::shared_ptr<const TIDVariableType> GetType() const { return type_; }

 private:
  std::shared_ptr<const TIDVariableType> type_;
  bool const_;
  bool reference_;
};

std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType type);
std::shared_ptr<TIDVariableType> DerivePointerFromType(const std::shared_ptr<TIDVariableType> & type);
std::shared_ptr<TIDVariableType> DeriveArrayFromType(const std::shared_ptr<TIDVariableType> & type);

class TID {
 public:
  TID() { AddScope(Lexeme() /* unused anyways :) */); }

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

  TIDVariable GetVariable(const std::wstring & name) const {
    for (size_t scope_index = nodes_.size() - 1; ~scope_index; --scope_index)
      if (nodes_[scope_index].variables_.count(name))
        return nodes_[scope_index].variables_.at(name);
    // Not found, returning empty variable
    return {};
  }
  TIDVariable GetVariable(const std::wstring & name) {
    return const_cast<const TID &>(*this).GetVariable(name);
  }

 public:
  void AddScope([[maybe_unused]] const Lexeme & current_lexeme) {
    nodes_.emplace_back();
  }

  void RemoveScope([[maybe_unused]] const Lexeme & current_lexeme) {
    if (nodes_.size() == 1)
      throw NoScopeAvailableError(current_lexeme);
    nodes_.pop_back();
  }

  void AddComplexStruct([[maybe_unused]] const Lexeme & current_lexeme, const std::wstring & name,
      const std::shared_ptr<TIDVariableType> & complex_struct) {
    if (std::dynamic_pointer_cast<TIDComplexVariableType>(complex_struct) == nullptr)
      throw NotComplexStruct(current_lexeme);
    if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
      throw ConflictingNames(current_lexeme);
    nodes_.back().complex_structs_[name] = complex_struct;
  }

  void AddVariable([[maybe_unused]] const Lexeme & current_lexeme, const std::wstring & name,
      const TIDVariable & variable) {
    if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
      throw ConflictingNames(current_lexeme);
    nodes_.back().variables_[name] = variable;
  }

 private:
  struct TIDNode {
    TIDNode() {}
    std::map<std::wstring, std::shared_ptr<TIDVariableType>> complex_structs_;
    std::map<std::wstring, TIDVariable> variables_;
  };
  std::vector<TIDNode> nodes_;
};
