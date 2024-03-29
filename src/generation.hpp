#pragma once

#include "TID.hpp"
#include <cstdlib>
#include <memory>

enum class NodeType : uint8_t {
  kOperand, kOperator, kReferenceOperand, kRelativeOperand
};

class RPNNode {
 public:
  virtual ~RPNNode() = default;
  NodeType GetNodeType() const { return type_; }
  virtual std::wstring ToString() const = 0;

 protected:
  RPNNode(NodeType type) : type_(type) {}

 private:
  NodeType type_;
};

class RPNOperand : public RPNNode {
 public:
  RPNOperand(uint64_t value) : RPNNode(NodeType::kOperand), value_(value) {}

  uint64_t GetValue() const { return value_; }
  std::wstring ToString() const override { return std::to_wstring(value_); }

 private:
  uint64_t value_;
};

// This is a node which is only used in compilation. It is used to reference other variables,
//  and it is replaced with RPNOperandNode as they are being merged
class RPNReferenceOperand : public RPNNode {
 public:
  RPNReferenceOperand(const std::wstring & name) : RPNNode(NodeType::kReferenceOperand), name_(name) {}

  std::wstring GetName() const { return name_; }
  std::wstring ToString() const override { return name_; }

 private:
  std::wstring name_;
};

class RPNRelativeOperand : public RPNNode {
 public:
  RPNRelativeOperand(uint64_t value) : RPNNode(NodeType::kRelativeOperand), value_(value) {}

  uint64_t GetValue() const { return value_; }
  std::wstring ToString() const override { return std::to_wstring(value_); }

 private:
  uint64_t value_;
};

// Item on stack:
// First 8 bytes - return pointer - when meeting "return" moves there. -1 if global
// Then space for return value (sizeof(return value) + 1), 1 first byte is 0 if anything was returned, else 1
//      other sizeof(return value) bytes are for return value
// Then go the function itself as well as all other arguments in order
// Then stack memory is up to RPN

enum class RPNOperatorType : uint8_t {
  // Internal Operators
  kLoad,      // Unary; Load data from $arg, size specified in value of size Size(type_); pushes value loaded to RPN
  kStoreDA,   // Binary: Store data $arg1 to $arg2, size specified in value of size Size(type_);
  kStoreAD,   // Binary: Store data $arg2 to $arg1, size specified in value of size Size(type_);
  kJmp,       // Unary: Jumps to $arg
  kCall,      // Unary: Same as jmp, but used to call function. Difference is that when run, $arg will be
              //        recorded as a function address to be used with kFuncSP
  kJz,        // Binary: Jumps to $arg2 if $arg1 is false
  kPush,      // Binary; Pushes to stack; $arg1 is size of memory to be reserved;
              //         $arg2 is address of function called; pushes new SP to RPN
  kPop,       // No args; Removes from stack
  kSP,        // No args; Pushes current SP to RPN (SP = stack pointer, pointer to base of last item put onto stack)
  kFromSP,    // Unary;  pushes SP + $arg to RPN
  kNew,       // Unary; allocates new memory of size ($arg as uint32) bytes on heap; pushes address to RPN
  kDelete,    // Binary; deletes memory at $arg1 of size ($arg2 as uint32)
  kRead,      // Unary; reads string to char[] at $arg ($arg is address of variable)
  kWrite,     // Unary; writes char[] that is at $arg ($arg is address of an array)
  kReturn,    // No args; Jumps back to return pointer (halts if return pointer is -1)
  kFuncSP,    // Unary; pushes (function that was called by jmp $arg)'s latest SP to RPN
  kDump,      // Unary; does nothing (takes $arg and disappears)
  kDuplicate, // Unary; does not consume argument, pushes its copy
  kSave,      // Unary; saves element pulled from stack in buffer
  kRestore,   // No args; pushes saved element in buffer; buffer is not cleared after;
              // by default is equal to 0
  kCopyFT,    // 3 args; copies from $arg1 into $arg2 chunk of size $arg3
  kCopyTF,    // 3 args; copies from $arg2 into $arg1 chunk of size $arg3
  kFill,      // Binary; fills chunk from $arg1 of size $arg2 with 0

  // Casting operators
  kToF64,   // Unary; pushes value casted from {type_} (int) to f64 to RPN
  kFromF64, // Unary; pushes value casted from f64 to {type_} (int) to RPN
  kToBool,  // Unary; pushes 0 if $arg == 0; 1 otherwise
  kToInt64, // Unary; pushed value casted from signed int to int64

  // Arithmetic operators ({operator} on {type} returns {type})
  kMinus,   // Unary; -$arg
  kTilda,   // Unary; ~$arg
  kAdd,     // Binary; $arg1 + $arg2
  kSubtract, // Binary; $arg1 - $arg2
  kMultiply, // Binary; $arg1 * $arg2
  kDivide, // Binary; $arg1 / $arg2
  kModulus, // Binary; $arg1 % $arg2
  kBitwiseShiftLeft, // Binary; $arg1 << $arg2
  kBitwiseShiftRight, // Binary; $arg1 >> $arg2
  kBitwiseAnd, // Binary; $arg1 & $arg2
  kBitwiseOr, // Binary; $arg1 | $arg2
  kBitwiseXor, // Binary; $arg1 ^ $arg2

  // Logical operators ({operator} on {type} returns bool)
  kInvert,  // Unary; !$arg
  kLess, // Binary; $arg1 < $arg2
  kMore, // Binary; $arg1 > $arg2
  kLessOrEqual, // Binary; $arg1 <= $arg2
  kMoreOrEqual, // Binary; $arg1 >= $arg2
  kEqual, // Binary; $arg1 == $arg2
  kNotEqual // Binary; $arg1 != $arg2
};

std::wstring ToString(RPNOperatorType);

class RPNOperator : public RPNNode {
 public:
  RPNOperator(RPNOperatorType type)
    : RPNNode(NodeType::kOperator), op_(type), type_(PrimitiveVariableType::kUnknown) {}
  RPNOperator(RPNOperatorType type, PrimitiveVariableType primitive_type)
    : RPNNode(NodeType::kOperator), op_(type), type_(primitive_type) {}

  RPNOperatorType GetOperatorType() const { return op_; }
  PrimitiveVariableType GetVariableType() const { return type_; }
  std::wstring ToString() const override { return ::ToString(op_); }

 private:
  RPNOperatorType op_;
  PrimitiveVariableType type_;
};

class RPN {
 public:
  RPN() {}

  const std::vector<std::shared_ptr<RPNNode>> & GetNodes() const { return nodes_; }
  std::vector<std::shared_ptr<RPNNode>> & GetNodes() { return nodes_; }

  void PushNode(const RPNOperand & operand) {
    nodes_.push_back(std::make_shared<RPNOperand>(operand));
  }
  void PushNode(const RPNReferenceOperand & operand) {
    nodes_.push_back(std::make_shared<RPNReferenceOperand>(operand));
  }
  void PushNode(const RPNRelativeOperand & operand) {
    nodes_.push_back(std::make_shared<RPNRelativeOperand>(operand));
  }
  void PushNode(const RPNOperator & op) {
    nodes_.push_back(std::make_shared<RPNOperator>(op));
  }
  void PushNode(std::shared_ptr<RPNNode> && node) {
    nodes_.push_back(std::move(node));
  }

 private:
  std::vector<std::shared_ptr<RPNNode>> nodes_;
};

PrimitiveVariableType GetTypeOfVariable(const std::shared_ptr<TIDVariableType> & type);

bool IsReference(const std::shared_ptr<TIDValue> & value);
bool IsReference(const std::shared_ptr<TIDVariableType> & type);

void AddReturn(RPN & rpn);
void LoadIfReference(const std::shared_ptr<TIDValue> & val, RPN & rpn);
void LoadIfReference(const std::shared_ptr<TIDVariableType> & val, RPN & rpn);
void LoadPointer(std::shared_ptr<TIDValue> & ptr, RPN & rpn);
