#pragma once

#include <vector>
#include "lexeme.hpp"

std::vector<Lexeme> PerformLexicalAnalysis(const std::wstring & code);
