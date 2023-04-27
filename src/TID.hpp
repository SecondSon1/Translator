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
  uint32_t GetSize() const { return size_; }

  bool IsConst() const { return const_; }
  bool IsReference() const { return ref_; }
  void SetConst(bool val) { const_ = val; }
  void SetReference(bool val) { ref_ = val; }

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
constexpr uint8_t kPrimitiveVariableTypeCount = 12;
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

PrimitiveVariableType FromWstringToPrimitiveType(const std::wstring & str);

class TIDPrimitiveVariableType : public TIDVariableType {
 private:
  struct Guard { explicit Guard(int) {} };

 public:
  TIDPrimitiveVariableType([[maybe_unused]] Guard _, PrimitiveVariableType & type)
    : TIDVariableType(VariableType::kPrimitive, GetSizeOfPrimitive(type)), type_(type) {}

  PrimitiveVariableType GetPrimitiveType() const { return type_; }

 private:
  friend std::shared_ptr<TIDVariableType> GetPrimitiveVariableType(PrimitiveVariableType);

 private:
  PrimitiveVariableType type_;
};

class TIDComplexVariableType : public TIDVariableType {
 public:
  TIDComplexVariableType(const std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> & contents)
    : TIDVariableType(VariableType::kComplex, 0), contents_(contents) {
    uint32_t our_size = 0;
    for (const auto & [name, type] : contents_)
      our_size += type->GetSize();
    SetSize(our_size);
  }

  const std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> & GetContents() const { return contents_; }

  std::shared_ptr<TIDVariableType> GetField(std::wstring & name) const;

 private:
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
    : TIDVariableType(VariableType::kArray, 8), value_(value) {}
  std::shared_ptr<TIDVariableType> GetValue() const { return value_; }

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

std::vector<std::shared_ptr<TIDVariableType>> GetDerivedTypes(const std::shared_ptr<TIDVariableType> & type);

class TID {
 public:
  TID() { AddScope(); }

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

  std::shared_ptr<const TIDVariableType> GetVariable(const std::wstring & name) const {
    for (size_t scope_index = nodes_.size() - 1; ~scope_index; --scope_index)
      if (nodes_[scope_index].variables_.count(name))
        return nodes_[scope_index].variables_.at(name);
    // Not found, returning empty variable
    return {};
  }
  std::shared_ptr<TIDVariableType> GetVariable(const std::wstring & name) {
    return std::const_pointer_cast<TIDVariableType>(const_cast<const TID &>(*this).GetVariable(name));
  }

 public:
  void AddScope();
  void RemoveScope();
  void AddComplexStruct(const Lexeme & lexeme, const std::wstring & name,
                        const std::shared_ptr<TIDVariableType> & complex_struct);
  void AddVariable(const Lexeme & lexeme, const std::wstring & name,
                   const std::shared_ptr<TIDVariableType> & variable);

 private:
  struct TIDNode {
    TIDNode() {}
    std::map<std::wstring, std::shared_ptr<TIDVariableType>> complex_structs_;
    std::map<std::wstring, std::shared_ptr<TIDVariableType>> variables_;
  };
  std::vector<TIDNode> nodes_;
};
