#include "logging.hpp"

#include <iostream>

#include "exceptions.hpp"
#include "warnings.hpp"
#include "lexeme.hpp"
#include "terminal_formatting.hpp"

std::wstring code_;
std::map<std::string, std::string> options_;

int64_t warningsNum = 0;

void log::init(const std::wstring &code, const std::map<std::string, std::string> & options) {
  code_ = code;
  options_ = options;
}

void printUnexpectedLexeme(const TranslatorError &err) {
  const UnexpectedLexeme *error = dynamic_cast<const UnexpectedLexeme *>(&err);
  if (error == nullptr) return;

  std::wcout << format::bright << " (" << format::reset;
  std::wcout << format::bright << "expected: " << color::blue << ToString(error->GetExpected()) << format::reset;
  std::wcout << format::bright << ", got: " << color::red << ToString(error->GetActual().GetType()) << format::reset;
  std::wcout << format::bright << ')' << format::reset;
}

void printUnknownOperator(const TranslatorError &err) {
  const UnknownOperator *error = dynamic_cast<const UnknownOperator *>(&err);
  if (error == nullptr) return;

  std::wcout << format::bright << " (" << format::reset;
  std::wcout << format::bright << color::blue << "TYPE_1" << format::reset;
  std::wcout << format::bright << " " << color::red << error->GetOperator().ToString() << " " << format::reset;
  std::wcout << format::bright << color::blue << "TYPE_2" << format::reset;
  std::wcout << format::bright << ')' << format::reset;
}

void printTypeMismatch(const TranslatorError &err) {
  const TypeMismatch *error = dynamic_cast<const TypeMismatch *>(&err);
  if (error == nullptr) return;

  std::wcout << format::bright << " (" << format::reset;
  std::wcout << format::bright << "expected: " << color::blue << error->GetTypeExpected()->ToString() << format::reset;
  std::wcout << format::bright << ", got: " << color::red << error->GetTypeGot()->ToString() << format::reset;
  std::wcout << format::bright << ')' << format::reset;
}

void printFunctionParameterListDoesNotMatch(const TranslatorError &err) {
  const FunctionParameterListDoesNotMatch *error = dynamic_cast<const FunctionParameterListDoesNotMatch *>(&err);
  if (error == nullptr) return;

  std::wcout << format::bright << " (" << format::reset;
  std::wcout << format::bright << "expected: " << color::blue;
  if (error->GetFunctionType()->GetParameters().size() == 0 && error->GetFunctionType()->GetDefaultParameters().size()) {
    std::wcout << "nothing";
  }
  else if (error->GetFunctionType()->GetParameters().size() > 0) {
    std::wcout << error->GetFunctionType()->GetParameters()[0]->ToString();
    for (size_t i = 1; i < error->GetFunctionType()->GetParameters().size(); ++i) {
      std::wcout << ", " << error->GetFunctionType()->GetParameters()[i]->ToString();
    }
    std::wcout << format::italic;
    for (size_t i = 0; i < error->GetFunctionType()->GetDefaultParameters().size(); ++i) {
      std::wcout << ", " << error->GetFunctionType()->GetDefaultParameters()[i]->ToString();
    }
  }
  else {
    std::wcout << format::italic;
    std::wcout << error->GetFunctionType()->GetDefaultParameters()[0]->ToString();
    for (size_t i = 1; i < error->GetFunctionType()->GetDefaultParameters().size(); ++i) {
      std::wcout << ", " << error->GetFunctionType()->GetDefaultParameters()[i]->ToString();
    }
  }
  std::wcout << format::reset;

  std::wcout << format::bright << ", got: " << color::red;
  if (error->GetProvided().size() == 0) {
    std::wcout << "nothing";
  }
  else {
    std::wcout << error->GetProvided()[0]->ToString();
    for (size_t i = 1; i < error->GetProvided().size(); ++i) {
      std::wcout << ", " << error->GetProvided()[i]->ToString();
    }
  }
  std::wcout << format::reset;
  std::wcout << format::bright << ')' << format::reset;
}

void log::error(const TranslatorError &error) {
  // Getting position in file
  size_t lineIndex = 1, columnIndex = 0, lineStartIndex = 0;
  for (size_t i = 0; i < error.GetIndex(); ++i, ++columnIndex) {
    if (code_[i] == '\n') {
      columnIndex = 0;
      ++lineIndex;
      lineStartIndex = i + 1;
    }
  }

  // Printing error info
  std::wcout << format::bright << lineIndex << ':' << columnIndex << ": ";
  std::wcout << color::red << "error: " << format::reset << format::bright << error.what() << format::reset;
  printUnexpectedLexeme(error);
  printUnknownOperator(error);
  printTypeMismatch(error);
  printFunctionParameterListDoesNotMatch(error);
  std::wcout << std::endl;

  // Getting error lexeme type
  size_t lexemeSize = 1;
  const SyntaxAnalysisError *syntaxError = dynamic_cast<const SyntaxAnalysisError *>(&error);
  const SemanticsAnalysisError *semanticsError = dynamic_cast<const SemanticsAnalysisError *>(&error);
  if (syntaxError != nullptr) {
    lexemeSize = syntaxError->GetActual().GetValue().size();
  } else if (semanticsError != nullptr) {
    lexemeSize = semanticsError->GetActual().GetValue().size();
  }

  // Printing line with error
  for (size_t i = lineStartIndex; i < code_.size() && code_[i] != '\n'; ++i) {
    if (i == error.GetIndex()) std::wcout << color::background::red << color::white;
    std::wcout << code_[i];
    if (i == error.GetIndex() + lexemeSize - 1) std::wcout << format::reset;
  }
  std::wcout << std::endl << std::endl;
}


void printDowncast(const TranslatorWarning &warn) {
  const Downcast *warning = dynamic_cast<const Downcast *>(&warn);
  if (warning == nullptr) return;

  std::wcout << format::bright << " (" << format::reset;
  std::wcout << format::bright << "from: " << color::blue << warning->GetFrom()->ToString() << format::reset;
  std::wcout << format::bright << ", to: " << color::red << warning->GetTo()->ToString() << format::reset;
  std::wcout << format::bright << ')' << format::reset;
}

void log::warning(const TranslatorWarning &warning) {
  if (options_["disableWarnings"] == "true") return;

  // Getting position in file
  size_t lineIndex = 1, columnIndex = 0, lineStartIndex = 0;
  for (size_t i = 0; i < warning.GetIndex(); ++i, ++columnIndex) {
    if (code_[i] == '\n') {
      columnIndex = 0;
      ++lineIndex;
      lineStartIndex = i + 1;
    }
  }

  // Printing warning info
  std::wcout << format::bright << lineIndex << ':' << columnIndex << ": ";
  std::wcout << color::blue << "warning: " << format::reset << format::bright << warning.what() << format::reset;
  printDowncast(warning);
  std::wcout << std::endl;

  // Getting warning lexeme type
  size_t lexemeSize = 1;
  const SyntaxAnalysisWarning *syntaxWarning = dynamic_cast<const SyntaxAnalysisWarning *>(&warning);
  const SemanticsAnalysisWarning *semanticsWarning = dynamic_cast<const SemanticsAnalysisWarning *>(&warning);
  if (syntaxWarning != nullptr) {
    lexemeSize = semanticsWarning->GetActual().GetValue().size();
  } else if (semanticsWarning != nullptr) {
    lexemeSize = semanticsWarning->GetActual().GetValue().size();
  }

  // Printing line with warning
  for (size_t i = lineStartIndex; i < code_.size() && code_[i] != '\n'; ++i) {
    if (i == warning.GetIndex()) std::wcout << color::background::blue << color::white;
    std::wcout << code_[i];
    if (i == warning.GetIndex() + lexemeSize - 1) std::wcout << format::reset;
  }
  std::wcout << std::endl << std::endl;
  ++warningsNum;
}

int64_t log::getWarningsNum() { return warningsNum; }
