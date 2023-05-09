#pragma once

#include <vector>
#include "lexeme.hpp"
#include "generation.hpp"

RPN PerformSyntaxAnalysis(const std::vector<Lexeme> & lexemes);
