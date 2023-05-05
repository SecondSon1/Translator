#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include "TID.hpp"
#include "lexeme.hpp"

class TranslatorWarning : public std::exception {
 public:
  TranslatorWarning(size_t index) : index_(index) {}

  size_t GetIndex() const { return index_; }

  virtual const char* what() const noexcept override {
    return "Unknown translator warning";
  }

 private:
  size_t index_;
};

// =================================
// === Lexical analysis warnings ===
// =================================

// Maybe PCF warning, no zachem

// ================================
// === Syntax analysis warnings ===
// ================================

class SyntaxAnalysisWarning : public TranslatorWarning {
  public:
  SyntaxAnalysisWarning(const Lexeme & actual) :
      TranslatorWarning(actual.GetIndex()), actual_(actual) {}

  Lexeme GetActual() const { return actual_; }

  const char* what() const noexcept override {
    return "Unknown warning at syntax analysis stage";
  }

  private:
  Lexeme actual_;
};

// =================================
// === Semantics analysis errors ===
// =================================

class SemanticsAnalysisWarning : public TranslatorWarning {
 public:
  SemanticsAnalysisWarning(const Lexeme & actual) :
      TranslatorWarning(actual.GetIndex()), actual_(actual) {}

  Lexeme GetActual() const { return actual_; }

  const char* what() const noexcept override {
    return "Unknown warning at semantics analysis stage";
  }

 private:
  Lexeme actual_;
};

class Downcast : public SemanticsAnalysisWarning {
 public:
  Downcast(const Lexeme & lexeme, const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) :
    SemanticsAnalysisWarning(lexeme), from_(from), to_(to) {}

  std::shared_ptr<TIDVariableType> GetFrom() const { return from_; }

  std::shared_ptr<TIDVariableType> GetTo() const { return to_; }

  const char* what() const noexcept override {
    return "Downcast, can be data loses";
  }

  private:
  std::shared_ptr<TIDVariableType> from_, to_;
};
