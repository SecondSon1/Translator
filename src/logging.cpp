#include "logging.hpp"

#include <iostream>

#include "exceptions.hpp"
#include "lexeme.hpp"
#include "terminal_formatting.hpp"

void log::error(const std::wstring &code, const TranslatorError &error) {
  // Getting position in file
  size_t lineIndex = 1, columnIndex = 0, lineStartIndex = 0;
  for (size_t i = 0; i < error.GetIndex(); ++i, ++columnIndex) {
    if (code[i] == '\n') {
      columnIndex = 0;
      ++lineIndex;
      lineStartIndex = i + 1;
    }
  }

  // Printing error info
  std::wcout << format::bright << lineIndex << ':' << columnIndex << ": ";
  std::wcout << color::red << "error: " << format::reset << format::bright << error.what() << format::reset;

  // More info for UnexpectedLexeme
  const UnexpectedLexeme *unexpectedLexeme = dynamic_cast<const UnexpectedLexeme *>(&error);
  if (unexpectedLexeme != nullptr) {
    std::wcout << format::bright << " (";
    std::wcout << "expected: " << color::blue << ToString(unexpectedLexeme->GetExpected()) << format::reset;
    std::wcout << format::bright << ", got: " << color::red << ToString(unexpectedLexeme->GetActual().GetType())
               << format::reset;
    std::wcout << format::bright << ')' << format::reset;
  }
  std::wcout << std::endl;

  // Printing line with error
  size_t lexemeSize = 1;
  const SyntaxAnalysisError *syntaxError = dynamic_cast<const SyntaxAnalysisError *>(&error);
  const SemanticsAnalysisError *semanticsError = dynamic_cast<const SemanticsAnalysisError *>(&error);
  if (syntaxError != nullptr) {
    lexemeSize = syntaxError->GetActual().GetValue().size();
  } else if (semanticsError != nullptr) {
    lexemeSize = semanticsError->GetActual().GetValue().size();
  }

  for (size_t i = lineStartIndex; i < code.size() && code[i] != '\n'; ++i) {
    if (i == error.GetIndex()) std::wcout << color::background::red << color::white;
    std::wcout << code[i];
    if (i == error.GetIndex() + lexemeSize - 1) std::wcout << format::reset;
  }
  std::wcout << std::endl << std::endl;
}

void log::warning(const std::wstring &code, const std::vector<Lexeme> &lexemes, uint64_t ind) {
  // For testing
  size_t index = lexemes[ind].GetIndex();

  // Getting lexeme position in file
  size_t lineIndex = 1, columnIndex = 0, lineStartIndex = 0;
  for (size_t i = 0; i < index; ++i, ++columnIndex) {
    if (code[i] == '\n') {
      columnIndex = 0;
      ++lineIndex;
      lineStartIndex = i + 1;
    }
  }

  // Printing warning info
  std::wcout << format::bright << lineIndex << ':' << columnIndex << ": ";
  std::wcout << color::blue << "warning: " << format::reset;
  std::wcout << format::bright << "converting " << color::green << "string" << format::reset;
  std::wcout << format::bright << " to " << color::red << "bool" << format::reset;
  std::wcout << std::endl;

  // Printing line with error
  for (size_t i = lineStartIndex; i < code.size() && code[i] != '\n'; ++i) {
    if (i == index) std::wcout << color::background::blue << color::white;
    std::wcout << code[i];
    if (i == index + lexemes[ind].GetValue().size() - 1) std::wcout << format::reset;
  }
  std::wcout << std::endl << std::endl;
}
