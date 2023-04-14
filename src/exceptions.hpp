#pragma once

#include <stdexcept>
#include <string>
#include "lexeme.hpp"

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
  SemanticsAnalysisError(const Lexeme & lexeme) : TranslatorError(lexeme.GetIndex()) {}

  const char* what() const noexcept override {
    return "Unknown error at semantics analysis stage";
  }
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

class NoScopeAvailableError : public SemanticsAnalysisError {
 public:
  NoScopeAvailableError(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "TID scope is already empty";
  }
};

class NotComplexStruct : public SemanticsAnalysisError {
 public:
  NotComplexStruct(const Lexeme & lexeme) : SemanticsAnalysisError(lexeme) {}

  const char* what() const noexcept override {
    return "Argument was not a complex struct";
  }
};
