#pragma once

#include "exceptions.hpp"

namespace log {
  void error(const std::wstring &code, const TranslatorError &error);

  void warning(const std::wstring &code, const std::vector<Lexeme> &lexemes, uint64_t ind);

  int64_t getWarningsNum();
}
