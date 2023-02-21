#pragma once

#include "lexeme.hpp"
#include <vector>

std::vector<Lexeme> PerformLexicalAnalysis(const std::wstring & code);
