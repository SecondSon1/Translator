#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include "TID.hpp"
#include "lexeme.hpp"
#include "operators.hpp"


// ======================================
// === Our Errors (not user's code's) ===
// ======================================

class NoScopeAvailableError : public std::exception {
 public:
  NoScopeAvailableError() {}

  const char* what() const noexcept override {
    return "TID scope is already empty";
  }
};

class NotComplexStructError : public std::exception {
 public:
  NotComplexStructError() {}

  const char* what() const noexcept override {
    return "Argument was not a complex struct";
  }
};

class OperatorTypeMismatch : public std::exception {
 public:
  OperatorTypeMismatch(OperatorType expected, OperatorType got) : expected_(expected), got_(got) {}

  OperatorType GetExpected() const { return expected_; }
  OperatorType GetGot() const { return got_; }

  const char* what() const noexcept override {
    return "Incorrect getter type of operator called";
  }

 private:
  OperatorType expected_, got_;
};

class ExpectedFunction : public std::exception {
 public:
  ExpectedFunction(const std::shared_ptr<TIDVariableType> got) : got_(got) {}

  std::shared_ptr<TIDVariableType> GetGot() const { return got_; }

  const char* what() const noexcept override {
    return "Not a function was passed";
  }

 private:
  std::shared_ptr<TIDVariableType> got_;
};

class UnableToCast : public std::exception {
 public:
  UnableToCast(const std::shared_ptr<TIDValue> & value, const std::shared_ptr<TIDVariableType> & type)
    : value_(value), type_(type) {}

  std::shared_ptr<TIDValue> GetValue() const { return value_; }
  std::shared_ptr<TIDVariableType> GetType() const { return type_; }

  const char* what() const noexcept override {
    return "Unable cast given value to a given type";
  }
 private:
  std::shared_ptr<TIDValue> value_;
  std::shared_ptr<TIDVariableType> type_;
};

// =============================
// === Now for user's errors ===
// =============================

class TranslatorError : public std::exception {
 public:
  TranslatorError(size_t index) : index_(index) {}

  size_t GetIndex() const { return index_; }

  virtual const char* what() const noexcept override {
    return "Unknown translator error";
  }

 private:
  size_t index_;
};

// ===============================
// === Lexical analysis errors ===
// ===============================

class LexicalAnalysisError : public TranslatorError {
 public:
  LexicalAnalysisError(size_t index) : TranslatorError(index) {}

  const char* what() const noexcept override {
    return "Unknown error at lexical analysis stage";
  }
};

class StringNotEndedError : public LexicalAnalysisError {
 public:
  StringNotEndedError(size_t index) : LexicalAnalysisError(index) {}

  const char* what() const noexcept override {
    return "String literal not ended";
  }
};

class CommentNotEndedError : public LexicalAnalysisError {
 public:
  CommentNotEndedError(size_t index) : LexicalAnalysisError(index) {}

  const char* what() const noexcept override {
    return "Multiline comment not ended";
  }
};

// TODO: maybe move to other stage
class UnknownEscapeSequenceError : public LexicalAnalysisError {
 public:
  UnknownEscapeSequenceError(size_t index) : LexicalAnalysisError(index) {}

  const char* what() const noexcept override {
    return "Unknown escape sequence found in string";
  }
};

class NumberNotFinishedError : public LexicalAnalysisError {
 public:
  NumberNotFinishedError(size_t index) : LexicalAnalysisError(index) {}

  const char* what() const noexcept override {
    return "Number started but not finished";
  }
};

class UnknownLexeme : public LexicalAnalysisError {
 public:
  UnknownLexeme(size_t index, const std::wstring & value) : LexicalAnalysisError(index), value_(value) {}

  std::wstring GetValue() const { return value_; }

  const char* what() const noexcept override {
    return "Number started but not finished";
  }
 private:
  std::wstring value_;
};

// ==============================
// === Syntax analysis errors ===
// ==============================

class SyntaxAnalysisError : public TranslatorError {
 public:
  SyntaxAnalysisError(const Lexeme & actual) :
    TranslatorError(actual.GetIndex()), actual_(actual) {}

  Lexeme GetActual() const { return actual_; }

  const char* what() const noexcept override {
    return "Unknown error at syntax analysis stage";
  }

 private:
  Lexeme actual_;
};

class UnexpectedLexeme : public SyntaxAnalysisError {
 public:
  UnexpectedLexeme(const Lexeme & actual, LexemeType expected) :
      SyntaxAnalysisError(actual), expected_(expected) {}

  LexemeType GetExpected() const { return expected_; }

  const char* what() const noexcept override {
    return "Unexpected lexeme";
  }

 private:
  LexemeType expected_;
};

class ExpectedExpression : public SyntaxAnalysisError {
  public:
  ExpectedExpression(const Lexeme & lexeme) : SyntaxAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Expression expected";
  }
};

class ExpectedDefaultParameter : public SyntaxAnalysisError {
  public:
  ExpectedDefaultParameter(const Lexeme & lexeme) : SyntaxAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Expected default parameter";
  }
};

// =================================
// === Semantics analysis errors ===
// =================================

class SemanticsAnalysisError : public TranslatorError {
 public:
  SemanticsAnalysisError(const Lexeme & actual) :
      TranslatorError(actual.GetIndex()), actual_(actual) {}

  Lexeme GetActual() const { return actual_; }

  const char* what() const noexcept override {
    return "Unknown error at semantics analysis stage";
  }

 private:
  Lexeme actual_;
};

class UndeclaredIdentifier : public SemanticsAnalysisError {
 public:
  UndeclaredIdentifier(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Use of undeclared identifier";
  }
};

class ConflictingNames : public SemanticsAnalysisError {
 public:
  ConflictingNames(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Conflicting names for variable/struct";
  }
};

class UnknownOperator : public SemanticsAnalysisError {
 public:
  UnknownOperator(const Lexeme & lexeme, UnaryPrefixOperator op, const std::shared_ptr<TIDValue> & val)
    : SemanticsAnalysisError(lexeme), op_(op), lhs_(val) {}
  UnknownOperator(const Lexeme & lexeme, UnaryPostfixOperator op, const std::shared_ptr<TIDValue> & val)
    : SemanticsAnalysisError(lexeme), op_(op), lhs_(val) {}
  UnknownOperator(const Lexeme & lexeme, BinaryOperator op, const std::shared_ptr<TIDValue> & lhs,
      const std::shared_ptr<TIDValue> & rhs)
    : SemanticsAnalysisError(lexeme), op_(op), lhs_(lhs), rhs_(rhs) {}

  Operator GetOperator() const { return op_; }

  // JUST FOR CLARITY, so we don't get tangled in this

  // this function is called only and only when op is unary, else it will throw error
  std::shared_ptr<TIDValue> GetValue() const {
    if (op_.GetOperatorType() == OperatorType::kBinary)
      throw OperatorTypeMismatch(OperatorType::kUnaryPrefix, op_.GetOperatorType());
    return lhs_;
  }

  // these two are called when op is binary
  std::shared_ptr<TIDValue> GetLHSValue() const {
    if (op_.GetOperatorType() != OperatorType::kBinary)
      throw OperatorTypeMismatch(OperatorType::kBinary, op_.GetOperatorType());
    return lhs_;
  }

  std::shared_ptr<TIDValue> GetRHSValue() const {
    if (op_.GetOperatorType() != OperatorType::kBinary)
      throw OperatorTypeMismatch(OperatorType::kBinary, op_.GetOperatorType());
    return rhs_;
  }

  const char* what() const noexcept override {
    return "Operator does not operate on provided type(s)";
  }
 private:
  Operator op_;
  std::shared_ptr<TIDValue> lhs_, rhs_; // if op_ is unary type is in lhs_
};

class TypeNotIterable : public SemanticsAnalysisError {
 public:
  TypeNotIterable(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Provided type is not iterable (not an array)";
  }
};

class TypeNotIndexed : public SemanticsAnalysisError {
 public:
  TypeNotIndexed(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Provided type is not indexed (not an array)";
  }
};

class TypeNotCallable : public SemanticsAnalysisError {
 public:
  TypeNotCallable(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Provided type is not callable (not a function)";
  }
};

class TypeNoMembers : public SemanticsAnalysisError {
 public:
  TypeNoMembers(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Provided type does not have members, attempting member access";
  }
};

class TypeUnknownMember : public SemanticsAnalysisError {
 public:
  TypeUnknownMember(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Member does not exist on given type";
  }
};

class TypeMismatch : public SemanticsAnalysisError {
 public:
  TypeMismatch(const Lexeme & lexeme, const std::shared_ptr<TIDVariableType> & expected, const std::shared_ptr<TIDVariableType> & got)
    : SemanticsAnalysisError(lexeme), expected_(expected), got_(got) {}

  std::shared_ptr<TIDVariableType> GetTypeExpected() const { return expected_; }
  std::shared_ptr<TIDVariableType> GetTypeGot() const { return got_; }

  const char* what() const noexcept override {
    return "Provided type is mismatched";
  }

 private:
  std::shared_ptr<TIDVariableType> expected_, got_;
};

class LoopInstructionsOutsideOfLoop : public SemanticsAnalysisError {
 public:
  LoopInstructionsOutsideOfLoop(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Continue/break outside of loop";
  }
};

class VoidNotExpected : public SemanticsAnalysisError {
 public:
  VoidNotExpected(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Void is not applicable here";
  }
};

class FunctionParameterListDoesNotMatch : public SemanticsAnalysisError {
 public:
  FunctionParameterListDoesNotMatch(const Lexeme & lexeme,
      const std::shared_ptr<TIDFunctionVariableType> & function_type,
      const std::vector<std::shared_ptr<TIDVariableType>> & provided)
    : SemanticsAnalysisError(lexeme), function_type_(function_type), provided_(provided) {}

  std::shared_ptr<TIDFunctionVariableType> GetFunctionType() const { return function_type_; }
  std::vector<std::shared_ptr<TIDVariableType>> GetProvided() const { return provided_; }

  const char* what() const noexcept override {
    return "Calling a function with incorrect parameter list";
  }

 private:
  std::shared_ptr<TIDFunctionVariableType> function_type_;
  std::vector<std::shared_ptr<TIDVariableType>> provided_;
};

