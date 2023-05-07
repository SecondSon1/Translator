#pragma once

#include "TID.hpp"
#include <cstdlib>

enum class NodeType : uint8_t {
  kOperand, kOperator, kReferenceOperand
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

class RPNOperandNode : public RPNNode {
 public:
  RPNOperandNode(uint64_t value) : RPNNode(NodeType::kOperand), value_(value) {}

  uint64_t GetValue() const { return value_; }
  std::wstring ToString() const override { return std::to_wstring(value_); }

 private:
  uint64_t value_;
};

// This is a node which is only used in compilation. It is used to reference other variables,
//  and it is replaced with RPNOperandNode as they are being merged
class RPNReferenceOperandNode : public RPNNode {
 public:
  RPNReferenceOperandNode(const std::wstring & name) : RPNNode(NodeType::kReferenceOperand), name_(name) {}

  std::wstring GetName() const { return name_; }
  std::wstring ToString() const override { return name_; }

 private:
  std::wstring name_;
};

// Item on stack:
// First 8 bytes - return pointer - when meeting "return" moves there. -1 if global
// Then space for return value (sizeof(return value) + 1), 1 first byte is 0 if anything was returned, else 1
//      other sizeof(return value) bytes are for return value
// Then go the function itself as well as all other arguments in order
// Then stack memory is up to RPN

enum class RPNOperatorType : uint8_t {
  // Internal Operators
  kLoad,     // Unary; Load data from $arg, size specified in value of size Size(type_); pushes value loaded to RPN
  kStore,    // Binary: Store data $arg1 to $arg2, size specified in value of size Size(type_);
  kJmp,      // Unary: Jumps to $arg
  kCall,     // Unary: Same as jmp, but used to call function. Difference is that when run, $arg will be
             //        recorded as a function address to be used with kFuncSP
  kJz,       // Binary: Jumps to $arg2 if $arg1 is false
  kPush,     // Unary; Pushes to stack; makes $arg size for returned value; pushes new SP to RPN
  kPop,      // No args; Removes from stack
  kSP,       // No args; Pushes current SP to RPN (SP = stack pointer, pointer to base of last item put onto stack)
  kFromSP,   // Unary;  pushes SP + $arg to RPN
  kNew,      // Unary; allocates new memory of size ($arg as uint32) bytes on heap; pushes address to RPN
  kDelete,   // Binary; deletes memory at $arg1 of size ($arg2 as uint32)
  kReturn,   // No args; Jumps back to return pointer (halts if return pointer is -1)
  kFuncSP,   // Unary; pushes (function that was called by jmp $arg)'s latest SP to RPN
  kDump,     // Unary; does nothing (takes $arg and disappears)

  // Casting operators
  kF64to32,   // Unary; pushes value casted from f64 to f32 to RPN
  kF32to64,   // Unary; pushes value casted from f32 to f64 to RPN
  kToF64,   // Unary; pushes value casted from {type_} (int) to f64 to RPN
  kFromF64, // Unary; pushes value casted from f64 to {type_} (int) to RPN
  kToBool,  // Unary; pushes 0 if $arg == 0; 1 otherwise

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

class RPNOperatorNode : public RPNNode {
 public:
  RPNOperatorNode(RPNOperatorType type)
    : RPNNode(NodeType::kOperator), op_(type), type_(PrimitiveVariableType::kUnknown) {}
  RPNOperatorNode(RPNOperatorType type, PrimitiveVariableType primitive_type)
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

  void PushNode(const RPNOperandNode & operand) {
    nodes_.push_back(std::make_shared<RPNOperandNode>(operand));
  }
  void PushNode(const RPNReferenceOperandNode & operand) {
    nodes_.push_back(std::make_shared<RPNReferenceOperandNode>(operand));
  }
  void PushNode(const RPNOperatorNode & op) {
    nodes_.push_back(std::make_shared<RPNOperatorNode>(op));
  }

 private:
  std::vector<std::shared_ptr<RPNNode>> nodes_;
};

PrimitiveVariableType GetTypeOfVariable(const std::shared_ptr<TIDVariableType> & type);
