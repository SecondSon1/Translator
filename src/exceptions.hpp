#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include "TID.hpp"
#include "lexeme.hpp"
#include "operators.hpp"

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
  UnknownOperator(const Lexeme & lexeme, Operator op) : SemanticsAnalysisError(lexeme), op_(op) {}

  Operator GetOperator() const { return op_; }

  const char* what() const noexcept override {
    return "Operator does not operate on provided type(s)";
  }
 private:
  Operator op_;
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
