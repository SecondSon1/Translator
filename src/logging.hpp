#pragma once

#include "exceptions.hpp"
#include "warnings.hpp"

namespace log {
  void init(const std::wstring &code);

  void error(const TranslatorError &error);

  void warning(const TranslatorWarning &warning);

  int64_t getWarningsNum();
}
