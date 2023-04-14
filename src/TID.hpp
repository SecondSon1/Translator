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
  bool IsConst() const { return const_; }
  bool IsReference() const { return reference_; }
  uint32_t GetSize() const { return size_; }

  void SetConst(bool value) { const_ = value; }
  void SetReference(bool value) { reference_ = value; }

 protected:
  TIDVariableType(VariableType type, bool constant, bool ref, uint32_t size)
      : variable_type_(type), const_(constant), reference_(ref), size_(size) {}
  void SetSize(uint32_t size) { size_ = size; }

 private:
  VariableType variable_type_;
  bool const_;
  bool reference_;
  uint32_t size_;
};

enum class PrimitiveVariableType : uint8_t {
  kInt8, kInt16, kInt32, kInt64,
  kUint8, kUint16, kUint32, kUint64,
  kF32, kF64, kBool, kChar
};

uint32_t GetSizeOfPrimitive(const PrimitiveVariableType & type);

class TIDPrimitiveVariableType : public TIDVariableType {
 public:
  TIDPrimitiveVariableType(PrimitiveVariableType & type, bool constant, bool ref)
    : TIDVariableType(VariableType::kPrimitive, constant, ref, GetSizeOfPrimitive(type)), type_(type) {}
 private:
  PrimitiveVariableType type_;
};

class TIDComplexVariableType : public TIDVariableType {
 public:
  TIDComplexVariableType(bool constant, bool ref,
      const std::vector<std::shared_ptr<TIDVariableType>> & contents)
    : TIDVariableType(VariableType::kComplex, constant, ref, 0), contents_(contents) {
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
  TIDFunctionVariableType(bool constant, bool ref, const std::shared_ptr<TIDVariableType> & return_type,
      const std::vector<std::shared_ptr<TIDVariableType>> & parameters,
      const std::vector<std::shared_ptr<TIDVariableType>> & default_parameters)
    : TIDVariableType(VariableType::kFunction, constant, ref, 8), return_type_(return_type),
      parameters_(parameters), default_parameters_(default_parameters) {}

  std::shared_ptr<TIDVariableType> GetReturnType() const { return return_type_; }
  const std::vector<std::shared_ptr<TIDVariableType>> & GetParameters() const { return parameters_; }
  const std::vector<std::shared_ptr<TIDVariableType>> & GetDefaultParameters() const { return default_parameters_; }

 private:
  std::shared_ptr<TIDVariableType> return_type_;
  std::vector<std::shared_ptr<TIDVariableType>> parameters_, default_parameters_;
};

class TIDPointerVariableType : public TIDVariableType {
 public:
  TIDPointerVariableType(bool constant, bool ref, const std::shared_ptr<TIDVariableType> & pointing_to)
    : TIDVariableType(VariableType::kPointer, constant, ref, 8), value_(pointing_to) {}

  std::shared_ptr<TIDVariableType> GetValue() const { return value_; }

 private:
  std::shared_ptr<TIDVariableType> value_;
};

class TIDArrayVariableType : public TIDVariableType {
 public:
  TIDArrayVariableType(bool constant, bool ref, const std::shared_ptr<TIDVariableType> & value)
    : TIDVariableType(VariableType::kPointer, constant, ref, 8), value_(value) {}

  std::shared_ptr<TIDVariableType> GetValue() const { return value_; }

 private:
  std::shared_ptr<TIDVariableType> value_;
};

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
    return std::const_pointer_cast<TIDVariableType>(static_cast<const TID &>(*this).GetComplexStruct(name));
  }

  std::shared_ptr<const TIDVariableType> GetVariable(const std::wstring & name) const {
    for (size_t scope_index = nodes_.size() - 1; ~scope_index; --scope_index)
      if (nodes_[scope_index].variables_.count(name))
        return nodes_[scope_index].variables_.at(name);
    // Not found, returning empty pointer
    return {};
  }
  std::shared_ptr<TIDVariableType> GetVariable(const std::wstring & name) {
    return std::const_pointer_cast<TIDVariableType>(static_cast<const TID &>(*this).GetVariable(name));
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
      const std::shared_ptr<TIDVariableType> & variable) {
    if (nodes_.back().complex_structs_.count(name) || nodes_.back().variables_.count(name))
      throw ConflictingNames(current_lexeme);
    nodes_.back().variables_[name] = variable;
  }

 private:
  struct TIDNode {
    TIDNode() {}
    std::map<std::wstring, std::shared_ptr<TIDVariableType>> complex_structs_;
    std::map<std::wstring, std::shared_ptr<TIDVariableType>> variables_;
  };
  std::vector<TIDNode> nodes_;
};
